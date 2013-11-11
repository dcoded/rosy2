#ifndef __INCLUDE_ROSY_OUTBOX_H__
#define __INCLUDE_ROSY_OUTBOX_H__


#include <cassert>

#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>

#include <dcoady/thread.h>
#include <dcoady/circular_fifo.h>

namespace rosy {

class outbox : public dcoady::thread {

    private:
        dcoady::circular_fifo<std::string, 1024> queue_;
        int   socket_;
        std::string addr_;

        const static int SEND_TIMEOUT = 5000;
        bool timeout_;
        int client_;

    public:
        outbox(char* addr)
        : socket_(-1)
        , addr_(addr)
        , timeout_(false)
        , client_(-1)
        {
            socket_ = nn_socket (AF_SP, NN_PUSH);
            assert (socket_ >= 0);

            int timeout = SEND_TIMEOUT;
            nn_setsockopt(socket_, NN_SOL_SOCKET, NN_SNDTIMEO, &timeout, sizeof(timeout));
            
            connect();
            

            std::cout << "outbox ready\n";
        }

        std::string addr() const
        {
            return addr_;
        }

        void update_endpoint(std::string endpoint)
        {
            addr_ = endpoint.c_str();
        }

        bool connect (const char* addr = 0)
        {
            if(addr != 0)
               addr_ = addr;

            if (client_ >= 0)
            {
                std::cout << "deleting old endpoint\n";
                nn_shutdown(socket_, client_);
            }

            
            std::cout << "connected()\n";
            client_ = nn_connect (socket_, addr_.c_str ());

            return (client_ >= 0);
        }

        void* run()
        {
            while(true)
            {
                while ( !queue_.empty ())
                {
                    send_message(queue_.dequeue ());
                }
            }

            return NULL;
        }

        void push (std::string message)
        {
            queue_.enqueue(message);
        }

        void operator()(std::string message)
        {
            push (message);
        }

        size_t size() const
        {
            return queue_.size();
        }

        bool timeout (bool clear = false)
        {
            bool val = timeout_;

            if (clear)
                timeout_ = false;

            return val;
        }

private:

        void flush_buffer ()
        {

        }

        void send_message(std::string message)
        {
            int bytes = nn_send (socket_, message.data(), message.size(), 0 /* NN_DONTWAIT */ );
            if (bytes != static_cast<int> (message.size ()))
            {
                queue_.enqueue(message);
                timeout_ = true;
            }
        }
};

}
#endif