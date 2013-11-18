#include <rosy/inbox.h>

namespace rosy {

inbox::inbox(const char* addr)
: socket_(-1)
, addr_(addr)
{
    socket_ = nn_socket (AF_SP, NN_PULL);
    assert (socket_ >= 0);

    assert (nn_bind (socket_, addr) >= 0);

    std::cout << "[OK] inbox initialized\n";
}

std::string inbox::addr() const { return addr_; }
size_t      inbox::size() const { return queue_.size(); }

void* inbox::run()
{
    while(true)
        recv_message();

    nn_shutdown(socket_, 0);
    return NULL;
}

std::string inbox::pop ()
{
    std::string msgstr;

    if (queue_.size() > 0)
        msgstr = queue_.dequeue();

    return msgstr;
}

void inbox::recv_message()
{
    char* buffer = NULL;

    int bytes = nn_recv (socket_, &buffer, NN_MSG, 0 /* NN_DONTWAIT */);
    assert (bytes >= 0);

    std::string message = std::string(buffer, bytes);
    queue_.enqueue(message);
    nn_freemsg (buffer);

    std::set<inbox_event_listener*>::iterator it;
    for(it = listeners_.begin (); it != listeners_.end (); it++)
        (*it)->on_recv(message);
}


void inbox::add_listener(inbox_event_listener const* listener)
{
    inbox_event_listener* ear = const_cast<inbox_event_listener*>(listener);

    listeners_.insert(ear);
    ear->start ();
}

}