#ifndef __INCLUDE_ROSY_INBOX_H__
#define __INCLUDE_ROSY_INBOX_H__

#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>

#include <cassert>

#include <dcoady/thread.h>
#include <dcoady/circular_fifo.h>

namespace rosy {

class inbox : public dcoady::thread {

    private:
        dcoady::circular_fifo<std::string> queue_;
        int socket_;
        std::string addr_;

        const static int BACKLOG = 3;

    public:
        inbox(const char* addr)
        : socket_(-1)
        , addr_(addr)
        {
            socket_ = nn_socket (AF_SP, NN_PULL);
            assert (socket_ >= 0);

            assert (nn_bind (socket_, addr) >= 0);

            std::cout << "inbox ready\n";
        }

        std::string addr() const
        {
            return addr_;
        }

        void* run()
        {
            while(true)
            {
                recv_message();
            }

            nn_shutdown(socket_, 0);
            return NULL;
        }

        std::string pop ()
        {
            std::string obj;

            if(queue_.size() > 0)
            {
                obj = queue_.dequeue();
            }

            return obj;
        }

        size_t size() const
        {
            return queue_.size();
        }

private:

        void recv_message()
        {
            char* buffer = NULL;

            int bytes = nn_recv (socket_, &buffer, NN_MSG, 0 /* NN_DONTWAIT */);
            assert (bytes >= 0);

         //  std::cout << std::string(buffer, bytes) << "\n";
            queue_.enqueue(std::string(buffer, bytes));
            nn_freemsg (buffer);
        }
};

}
#endif