#ifndef __INCLUDE_ROSY_IPC_BROADCAST_H__
#define __INCLUDE_ROSY_IPC_BROADCAST_H__

#include <iostream>

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>

#include <dcoady/thread.h>

#include "inbox.h"
#include "outbox.h"

#include <dcoady/circular_fifo.h>




namespace rosy {

class ipc_broadcast : public inbox_event_listener {
private:
    const char* addr_;

    inbox* inbox_;

    dcoady::circular_fifo<std::string> queue_;

    int socket_;

public:
    ipc_broadcast(const char* ipc, inbox& inbox);

    void* run();

    void on_recv(std::string msg);

    void broadcast ();
};




}

#endif