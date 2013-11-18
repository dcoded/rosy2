
#include <rosy/tcp_client.h>

namespace rosy {


tcp_client::tcp_client()
{
}

void tcp_client::reset_ ()
{
    flags_ &= 0x0;
    error_ &= 0x0;

    notify ();
}

void tcp_client::endpoint (const char* endpoint)
{
    int i,k;
    for(i = 0; endpoint[i]   != 0 && endpoint[i] != ':'; i++) ;
    for(k = 1; endpoint[k+i] != 0;                       k++) ;

    host_ = std::string (endpoint    , i);
    port_ = std::string (endpoint+i+1, k);

    flags_ |= F_RESET;

    std::cout << "CLIENT changing endpoint\n";
    notify ();
}

void tcp_client::connect_ ()
{
    std::cout << "Connecting to " << host_ << ":" << port_ << "\n";

    int status;
    struct addrinfo  host_info;      // The struct that getaddrinfo() fills up with data.
    struct addrinfo* host_info_list; // Pointer to the to the linked list of host_info's.

    // The MAN page of getaddrinfo() states "All  the other fields in the structure pointed
    // to by hints must contain either 0 or a null pointer, as appropriate." When a struct 
    // is created in C++, it will be given a block of memory. This memory is not necessary
    // empty. Therefor we use the memset function to make sure all fields are NULL.     
    memset (&host_info, 0, sizeof host_info);

    host_info.ai_family   = AF_UNSPEC;   // IP version not specified. Can be both.
    host_info.ai_socktype = SOCK_STREAM; // Use SOCK_STREAM for TCP or SOCK_DGRAM for UDP.

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
        error_ |= E_SOCK;


    //nn_setsockopt(socket_, NN_SOL_SOCKET, NN_SNDTIMEO, &timeout, sizeof(timeout));

    status = connect (socket_, host_info_list->ai_addr,
                               host_info_list->ai_addrlen);

    if (status == -1)
        error_ |= E_CONN;
}


void tcp_client::write (std::string message)
{
    queue_.enqueue (message);
    flags_ |= F_WRITE;
    notify ();
}


void tcp_client::write_ ()
{
    int bytes_transferred;

    if (queue_.size () > 0)
    {
        std::string msg   = queue_.front ();
        std::cout << "Sending message: " << msg << "\n";
        bytes_transferred = send (socket_, msg.c_str (), msg.size (), MSG_DONTWAIT);

        if (bytes_transferred == static_cast<int> (msg.size ()))
        {
            queue_.dequeue ();
            std::cout << "Message sent successfully\n";
        }
        else if (errno_failure ())
            error_ |= E_SEND;
        else
            flags_ |= F_AGAIN;
    }
}

bool tcp_client::errno_failure ()
{
    perror("CLIENT ");
    switch(errno)
    {
        case EAGAIN     : return false;
        break;
        default         : return true;
    }
}


void* tcp_client::run ()
{
    while(1)
    {
        usleep(10000);

        if (flags_ & F_RESET)
            state_ = RESET;

        switch(state_)
        {
            case INIT:  //std::cout << "CLIENT INIT\n";
                state_ = RESET;
            break;

            case RESET: std::cout << "CLIENT RESET\n";
            {
                state_ = CONNECT;

                reset_ ();
                flags_ &= ~F_RESET;
            }
            break;

            case CONNECT: std::cout << "CLIENT CONNECT\n";
            {
                connect_ ();

                if (error_)
                    state_ = ERROR;
                else
                    state_ = WAIT;
            }
            break;

            case WAIT: //std::cout << "CLIENT WAIT\n";
            {
                if (flags_ & F_RESET)
                    state_ = RESET;

                else if (queue_.size () > 0)
                    state_ = WRITE;

                else if (error_)
                    state_ = ERROR;

                else wait ();
            }
            break;

            case WRITE: //std::cout << "CLIENT WRITE\n";
            {
                write_ ();

                if (flags_ & F_AGAIN)
                    state_ = WRITE;

                else if (error_)
                    state_ = ERROR;

                else
                    state_ = WAIT;
            }
            break;

            case ERROR: //std::cout << "CLIENT ERROR\n";
            {
                state_ = RESET;
            }
            break;
        }
    }

    return NULL;
}



const std::string tcp_client::addr () const
{
    return host_ + ":" + port_;
}

}


