#ifndef __INCLUDE_ROSY_OUTBOX_H__
#define __INCLUDE_ROSY_OUTBOX_H__

#include <iostream>
#include <cassert>

#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>

#include <dcoady/thread.h>
#include <dcoady/circular_fifo.h>

namespace rosy {

class outbox : public dcoady::thread {

    private:
        enum state { IDLE, CONNECT, READY, SEND };
        state state_;

        dcoady::circular_fifo<std::string, 1024> queue_;
        int   socket_;
        std::string addr_;

        const static int SEND_TIMEOUT = 5000;
        bool timeout_;
        int  client_;

    public:
        outbox(char* addr);

        std::string addr() const;

        bool connect (const char* addr = 0);

        void* run();

        void push (std::string message);

        void operator()(std::string message);

        size_t size() const;

        bool timeout (bool clear = false);

        bool ready ();

private:

        void send_message(std::string message);
};

}
#endif