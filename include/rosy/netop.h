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

class netop : public tcp_server_event_listener
            , public tcp_client_event_listener {
    private:

        enum  state
        {
            INIT,
            JOIN,
            READY,
            ADD_PEER,
            SET_PEER,
            DEL_PEER,
            RECOVER,
            UNKNOWN
        };
        state curr_state_,
              next_state_;

        tcp_client* client_;
        tcp_server* server_;

        std::set<std::string> peers_;

        dcoady::circular_fifo<std::string> queue_;

        std::string uuid_;


    public:
        netop(tcp_server& server, tcp_client& client);


        void* run ();
        void  on_recv (std::string message);
        void  on_timeout ();

    private:
        void  advance_ ();
        void  execute_ ();

        void  relink_ ();
        void  remove_ (std::string addr);
        void  print_() const;
        std::string next_ (std::string addr);
};


}

#endif