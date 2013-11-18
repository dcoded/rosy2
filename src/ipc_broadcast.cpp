#include <rosy/ipc_broadcast.h>


namespace rosy {

ipc_broadcast::ipc_broadcast(const char* ipc, inbox& inbox)
: addr_(ipc)
, inbox_ (&inbox)
{
    socket_ = nn_socket (AF_SP, NN_PUB);
    assert (socket_ >= 0);
    assert (nn_bind (socket_, addr_) >= 0);

    inbox_->add_listener (this);
}

void* ipc_broadcast::run ()
{
    std::cout << "[OK] ipc broadcast initialized\n";
    while(1)
    {
        broadcast ();
        wait();
    }

    nn_shutdown (socket_, 0);
    return NULL;
}

void ipc_broadcast::broadcast ()
{
    while(!queue_.empty ())
    {
        std::string msg = queue_.dequeue ();

        int bytes = nn_send (socket_, msg.data (), msg.size (), 0);
        if (bytes != static_cast<int>(msg.size ()))
        {
            queue_.enqueue (msg);
        }
        else
        {
            //std::cout << "[PUB] " << msg << "\n";
        }
    }
}

void ipc_broadcast::on_recv(std::string msg)
{
    //message::type type = static_cast<message::type> (msg[0]);
    queue_.enqueue (msg);
    notify();
}

}