#ifndef __INCLUDE_NETOP_H__
#define __INCLUDE_NETOP_H__

#include <string>
#include <map>

#include "tcp_client.h"
#include "tcp_server.h"

#include "lite_message.h"
#include "uuid.h"

#include <dcoady/circular_fifo.h>

namespace rosy {

class netop : public tcp_server_event_listener {
    private:

        enum  state { INIT, JOINING, JOINED, READY, ADD_PEER, DEL_PEER, RELINK };
        state state_;

        tcp_client* client_;
        tcp_server* server_;

        std::set<std::string> peers_;

        dcoady::circular_fifo<std::string> queue_;

        std::string uuid_;


    public:
        netop(tcp_server& server, tcp_client& client);


        void* run ();
        void  on_recv (std::string message);

    private:

        void  relink_ ();
        void  print_() const;
};


}

#endif