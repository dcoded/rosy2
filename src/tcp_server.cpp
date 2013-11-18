#include <rosy/tcp_server.h>

namespace rosy {


tcp_server::tcp_server()
{
    
    // /* setup signal handler */
    // sigemptyset(&sig_act_.sa_mask);
    // sig_act_.sa_flags     = SA_SIGINFO;
    // sig_act_.sa_sigaction = &tcp_server::signal_;


    // /* link request for signal handler */
    // aio_req_.aio_sigevent.sigev_notify = SIGEV_SIGNAL;
    // aio_req_.aio_sigevent.sigev_signo  = SIGIO;

    // aio_req_.aio_sigevent.sigev_value.sival_ptr = this;

    // /* map handler for SIGIO */
    // sigaction(SIGIO, &sig_act_, NULL);

    // std::cout << "Signal handler ready\n";
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
    static const int yes = 1;
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

    freeaddrinfo(host_info_list);

    if (socket_ == -1)
    {
        perror("SERVER: ");
        error_ |= E_SOCK;
    }


    status = fcntl(socket_, F_SETFL, O_NONBLOCK);
    
    if (status != 0)
    {
        perror("SERVER: ");
        error_ |= E_FCTL;
    }


    status = setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

    if (status != 0)
    {
        perror("SERVER: ");
        error_ |= E_OPTN;
    }


    status = bind (socket_, host_info_list->ai_addr,
                            host_info_list->ai_addrlen);

    if (status != 0)
    {
        perror("SERVER: ");
        error_ |= E_BIND;
    }


    status = listen(socket_, 5);
    
    if (status != 0)
    {
        perror("SERVER: ");
        error_ |= E_LISN;
    }
}

void tcp_server::accept_ ()
{
    connection client;

    client.socket = accept4 (socket_, &client.address,
                                      &client.len,
                                      SOCK_NONBLOCK);

    //std::cout << "accepted " << client.socket << "\n";
    if(errno_failure())
        error_ |= E_ACPT;
    else if (client.socket >= 0)
        clients_.push_back(client);
    //else
    //    flags_ |= F_AGAIN;
}

void tcp_server::read_ ()
{
    static char buffer [1024];
    int         bytes_transferred;

    std::vector<connection>::iterator it;
    for(it = clients_.begin (); it != clients_.end (); it++)
    {
        //std::cout << "Polling client " << it->socket << "\n";
        bytes_transferred = recv (it->socket, buffer, 1024, 0);

        if (bytes_transferred > 0)
        {
            //queue_.enqueue (std::string (buffer, bytes_transferred));
            distribute_ (std::string (buffer, bytes_transferred));
        }

        else if (errno_failure ())
        {
            close (it->socket);
            clients_.erase (it);
        }
    }
}

void tcp_server::distribute_ (std::string message) const
{
    std::set<tcp_server_event_listener*>::iterator it;
    for(it = listeners_.begin (); it != listeners_.end (); it++)
        (*it)->on_recv(message);
}

bool tcp_server::errno_failure ()
{
    switch(errno)
    {
        case EAGAIN     : return false;
        break;
        default         : return true;
    }
}


void* tcp_server::run ()
{
    while(1)
    {
        usleep(50);
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
                    state_ = ACCEPT;
            }
            break;

            case WAIT: std::cout << "SERVER WAIT\n";
            {
                // if (flags_ & F_SIGIN)
                //     state_ = ACCEPT;
                // else
                // {
                //     state_ = WAIT;
                //     wait ();
                // }

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

            case READ: //std::cout << "SERVER READ\n";
            {
                read_ ();

                if (flags_ & F_AGAIN)
                    state_ = READ;

                else if (error_)
                    state_ = ERROR;

                else
                    state_ = ACCEPT;
            }
            break;

            case ERROR: //std::cout << "SERVER ERROR\n";
            {
                std::cout << "code = " << error_ << "\n";
                state_ = RESET;
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

const size_t tcp_server::size() const
{
    return queue_.size ();
}

// void tcp_server::signal_ (int sig, siginfo_t *siginfo, void *context)
// {
//     tcp_server* this_ = reinterpret_cast<tcp_server*> (context);
//     std::cout << "SIGIN\n";
// }

const bool tcp_server::ready () const
{
    return (state_ & (ACCEPT | READ)) != 0;
}


}


