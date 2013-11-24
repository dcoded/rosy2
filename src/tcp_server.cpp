#include <rosy/tcp_server.h>

namespace rosy {

std::list<tcp_server*> tcp_server::instances_;


tcp_server::tcp_server()
{
    /* setup signal handler */
    sigemptyset(&sig_act_.sa_mask);
    sig_act_.sa_flags     = SA_SIGINFO;
    sig_act_.sa_sigaction = &tcp_server::signal_;


    // /* link request for signal handler */
    // aio_req_.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    // aio_req_.aio_sigevent.sigev_signo  = SIGIO;

    // aio_req_.aio_sigevent.sigev_value.sival_ptr = this;

    /* map handler for SIGIO */
    assert (sigaction(SIGIO, &sig_act_, NULL) >= 0);
    instances_.push_back (this);
}

void tcp_server::reset_ ()
{
    flags_ &= 0x0;
    error_ &= 0x0;

    clients_.clear ();
    
    notify ();
}

void tcp_server::endpoint (const char* endpoint)
{
    int i,k;
    for(i = 0; endpoint[i]   != 0 && endpoint[i] != ':'; i++) ;
    for(k = 1; endpoint[k+i] != 0;                       k++) ;

    host_ = std::string (endpoint    , i);
    port_ = std::string (endpoint+i+1, k);

    flags_ |= F_RESET;
    notify ();
}

void tcp_server::bind_ ()
{
    int status;
    const int yes = 1;
    struct addrinfo  host_info;      // The struct that getaddrinfo() fills up with data.
    struct addrinfo* host_info_list; // Pointer to the to the linked list of host_info's.

    // The MAN page of getaddrinfo() states "All  the other fields in the structure pointed
    // to by hints must contain either 0 or a null pointer, as appropriate." When a struct 
    // is created in C++, it will be given a block of memory. This memory is not necessary
    // empty. Therefor we use the memset function to make sure all fields are NULL.     
    memset (&host_info, 0, sizeof host_info);

    host_info.ai_family   = AF_UNSPEC;   // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.
    host_info.ai_flags    = AI_PASSIVE; 

    // Now fill up the linked list of host_info structs with google's address information.
    status = getaddrinfo (host_.c_str (), port_.c_str (), &host_info, &host_info_list);
    // getaddrinfo returns 0 on success, or some other value when an error occured.
    // (translated into human readable text by the gai_gai_strerror function).   
    if (status != 0)
        error_ |= E_ADDR;

    socket_ = socket (host_info_list->ai_family,
                      host_info_list->ai_socktype, 
                      host_info_list->ai_protocol);


    if (socket_ == -1)
        error_ |= E_SOCK;

    if (fcntl (socket_, F_SETOWN, getpid ()) == -1)
        error_ |= E_FCTL;

    
    if (fcntl (socket_, F_SETFL, O_NONBLOCK | FASYNC) == -1)
        error_ |= E_FCTL;


    status = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    if (status != 0)
        error_ |= E_OPTN;


    status = bind (socket_, host_info_list->ai_addr,
                            host_info_list->ai_addrlen);

    if (status != 0)
        error_ |= E_BIND;

    
    if (listen(socket_, 5) == -1)
        error_ |= E_LISN;


    freeaddrinfo(host_info_list);
}

void tcp_server::accept_ ()
{
    connection client;

    client.socket = accept4 (socket_, &client.address,
                                      &client.len,
                                      SOCK_NONBLOCK);

    //std::cout << "accepted " << client.socket << "\n";
    if (errno_failure_ ())
    {
        std::cout << "errno = " << errno << "\n";
        perror ("SERVER: ");
        error_ |= E_ACPT;
    }
    else if (client.socket >= 0)
    {
        if (fcntl (client.socket, F_SETOWN, getpid ()) == -1
        ||  fcntl (client.socket, F_SETFL, O_NONBLOCK | FASYNC) == -1)
        {
            perror("CONNECTION: ");
            error_ |= E_FCTL;
        }
        else clients_.push_back (client);
    }
    //else
    //    flags_ |= F_AGAIN;
}

void tcp_server::read_ ()
{
    static char buffer [1024];
    int         bytes_transferred;

    std::list<connection>::iterator it, it2;
    for(it = clients_.begin (); it != clients_.end (); it++)
    {
        //std::cout << "reading " << it->socket << "\n";
        bytes_transferred = recv (it->socket, buffer, 1024, 0);

        if (bytes_transferred > 0)
        {
            distribute_ (std::string (buffer, bytes_transferred));
            for (it2 = clients_.begin (); it2 != clients_.end (); it2++)
                 it2->queue.enqueue (std::string (buffer, bytes_transferred));
        }

        else if (errno == EINTR)
            flags_ |= F_AGAIN;

        else if (bytes_transferred == 0 || errno_failure_ ())
        {
            //std::cout << "closing " << it->socket << "\n";
            close (it->socket);
            clients_.erase (it++);
        }
    }
}

void tcp_server::broadcast_ ()
{
    std::string msg;
    std::list<connection>::iterator it;

    for (it = clients_.begin (); it != clients_.end (); it++)
    {
        if (!it->queue.empty ())
        {
            msg = it->queue.front ();

            int bytes = send (it->socket, msg.c_str (), msg.size (), MSG_DONTWAIT);

            if (bytes == static_cast<int> (msg.size ()))
            {
                std::cout << "[" << it->socket << "] " << msg << "\n";
                it->queue.dequeue ();
            }

            else if (bytes == 0 || errno_failure_ ())
            {
                //std::cout << "closing " << it->socket << "\n";
                close (it->socket);
                clients_.erase (it++);
            }

            else
            {
                //std::cout << "didnt send it...\n";
            }

        }
    }
}

void tcp_server::distribute_ (std::string message) const
{
    std::set<tcp_server_event_listener*>::iterator it;

    for(it = listeners_.begin (); it != listeners_.end (); it++)
        (*it)->on_recv(message);
}

bool tcp_server::errno_failure_ () const
{
    return (errno & ~EAGAIN) > 0;
}


void* tcp_server::run ()
{
    while(1)
    {
        switch(state_)
        {
            case INIT:  std::cout << "SERVER INIT\n";
                state_ = RESET;
            break;

            case RESET: std::cout << "SERVER RESET\n";
            {
                state_ = BIND;

                reset_ ();
                flags_ &= ~F_RESET;
            }
            break;

            case BIND: std::cout << "SERVER BIND\n";
            {
                bind_ ();

                if (error_)
                    state_ = ERROR;
                else
                    state_ = WAIT;
            }
            break;

            case WAIT: //std::cout << "SERVER WAIT\n";
            {
                wait ();
                state_ = ACCEPT;
            }
            break;

            case ACCEPT: //std::cout << "SERVER ACCEPT\n";
            {
                accept_ ();
                if (error_)
                    state_ = ERROR;
              //  else if (flags_ & F_AGAIN)
               //     state_ = ACCEPT;
                else
                    state_ = READ;
            }
            break;

            case READ: std::cout << "SERVER READ\n";
            {
                read_ ();

                if (flags_ & F_AGAIN)
                {
                    state_ = READ;
                    flags_ &= ~F_AGAIN;
                }
                else if (error_)
                    state_ = ERROR;

                else
                    state_ = BCAST;
            }
            break;

            case BCAST: std::cout << "SERVER BCAST\n";
            {
                broadcast_ ();
                state_ = WAIT;
            }
            break;

            case ERROR: //std::cout << "SERVER ERROR\n";
            {
                std::cout << "code = " << error_ << "\n";
                state_ = RESET;
                sleep(2);
            }
            break;
        }
    }

    return NULL;
}



const std::string tcp_server::addr () const
{
    return std::string(host_ + ":" + port_).substr(0,15);
}

void tcp_server::add_listener(tcp_server_event_listener const* listener)
{
    tcp_server_event_listener* ear = const_cast<tcp_server_event_listener*>(listener);

    listeners_.insert(ear);
    ear->start ();
}

// const std::string tcp_server::pop  ()
// {
//     std::string msgstr;

//     if (queue_.size() > 0)
//         msgstr = queue_.dequeue();

//     return msgstr;
// }

// void tcp_server::signal_ (int sig, siginfo_t *siginfo, void *context)
// {
//     tcp_server* this_ = reinterpret_cast<tcp_server*> (context);
//     std::cout << "SIGIN\n";
// }

const bool tcp_server::ready () const
{
    return (state_ & (WAIT | ACCEPT | READ)) != 0;
}


// bool tcp_server::is_http_ (const char* buffer) const
// {
//     return
//     (
//         strncmp ("GET",  buffer, 3) == 0
//     ||  strncmp ("PUT",  buffer, 3) == 0
//     ||  strncmp ("POST", buffer, 4) == 0
//     ||  strncmp ("HEAD", buffer, 4) == 0
//     );
// }

// void tcp_server::http_stats_ (std::list<connection>::iterator& client )
// {

//     std::stringstream ss;

//     ss << "HTTP/1.x 200 OK"                         << "\r\n"
//        << "Connection: close"                       << "\r\n"
//        << "Content-Type: text/html; charset=UTF-8"  << "\r\n"
//        << "Content-Length : " << addr ().size() + 7 << "\r\n"
//        << "\r\n"
//        << addr () << " is up!";

//     send  (client->socket, ss.str().c_str (), ss.str().size (), 0);
//     //close (client->socket);

//     //clients_.erase (client);
// }


void tcp_server::signal_ (int sig, siginfo_t *siginfo, void *context)
{
    std::list<tcp_server*>::iterator it;
    for(it = instances_.begin (); it != instances_.end (); it++)
        (*it)->notify ();
}


}


