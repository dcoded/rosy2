
#include <rosy/outbox.h>

namespace rosy {


outbox::outbox(char* addr)
: state_(IDLE)
, socket_(-1)
, addr_(addr)
, timeout_(false)
, client_(-1)
{
    socket_ = nn_socket (AF_SP, NN_PUSH);
    assert (socket_ >= 0);

    int timeout = SEND_TIMEOUT;
    nn_setsockopt(socket_, NN_SOL_SOCKET, NN_SNDTIMEO, &timeout, sizeof(timeout));
    
    connect ();
    

    std::cout << "[OK] outbox initialized\n";
}

std::string outbox::addr() const { return addr_; }
size_t      outbox::size() const { return queue_.size(); }

bool outbox::connect (const char* addr)
{
    state_ = CONNECT;

    if(addr != 0)
       addr_ = addr;

    if (client_ >= 0)
    {
        std::cout << "deleting old endpoint\n";
        nn_shutdown(socket_, client_);
    }

    client_ = nn_connect (socket_, addr_.c_str ());

    if (client_ >= 0)
        state_ = READY;

    return ready ();
}

void* outbox::run()
{
    while(true)
    {
        while (state_ == READY && !queue_.empty ())
        {
            state_ = SEND;
            send_message(queue_.front ());
        }

        state_ = READY;
        wait();
    }

    return NULL;
}

void outbox::push (std::string message)
{
    queue_.enqueue(message);
    notify();
}

void outbox::operator()(std::string message)
{
    push (message);
}

bool outbox::timeout (bool clear)
{
    bool val = timeout_;

    if (clear)
        timeout_ = false;

    return val;
}

void outbox::send_message(std::string message)
{
    std::cout << "sending...\n";
    int bytes = nn_send (socket_, message.data(), message.size(), 0 /* NN_DONTWAIT */ );
    if (bytes != static_cast<int> (message.size ()))
        timeout_ = true;
    
    else queue_.dequeue ();

    std::cout << "done sending\n";
}

bool outbox::ready ()
{
    return state_ == READY;
}

}