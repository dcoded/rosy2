#ifndef __INCLUDE_ROSY_INBOX_H__
#define __INCLUDE_ROSY_INBOX_H__


#include <iostream>
#include <cassert>

#include <set>

#include <nanomsg/nn.h>
#include <nanomsg/pipeline.h>

#include <dcoady/thread.h>
#include <dcoady/circular_fifo.h>

namespace rosy {

class inbox_event_listener : public dcoady::thread {
public:
    virtual void on_recv(std::string) = 0;
};

class inbox : public dcoady::thread {

    private:
        dcoady::circular_fifo<std::string> queue_;
        int socket_;
        std::string addr_;

        std::set<inbox_event_listener*> listeners_;

        const static int BACKLOG = 3;

    public:
        inbox(const char* addr);

        std::string addr() const;

        void* run();

        std::string pop ();

        size_t size() const;

        void add_listener(inbox_event_listener const* listener);

private:

        void recv_message();
};

}
#endif