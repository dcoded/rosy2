#ifndef __INCLUDE_ROSY_tcp_client_H__
#define __INCLUDE_ROSY_tcp_client_H__

#include <iostream>
#include <cassert>

#include <set>
#include <cstdio>
#include <memory.h>
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions
#include <cerrno>

#include <dcoady/thread.h>
#include <dcoady/circular_fifo.h>

namespace rosy {

class tcp_client_event_listener {
public:
    virtual void on_timeout() = 0;
};

class tcp_client : public dcoady::thread {

    private:

        /* FSM State Variables */
        enum  state { INIT, RESET, CONNECT, WAIT, WRITE, ERROR };
        state state_;
      
        unsigned int flags_;
        enum         flags { F_RESET = 0x1, F_AGAIN = 0x2,
                             F_WRITE = 0x8 };

        unsigned int error_;
        enum         error { E_ADDR  = 0x1, E_SOCK  = 0x2,
                             E_CONN  = 0x4, E_SEND  = 0x8 };


        /* Thread safe FIFO queue */
        dcoady::circular_fifo<std::string, 1024> queue_;

        /* Socket details */
        int  socket_; // socket id
        int  client_; // connection id

        /* config params */
        std::string host_;
        std::string port_;

        /* event listeners */
        std::set<tcp_client_event_listener*> listeners_;

    public:
        tcp_client ();
        void  write (std::string message);
        void  endpoint (const char* endpoint);
        void* run ();

        const std::string addr () const;
        void  add_listener (tcp_client_event_listener const* listener);

    private:
        void  reset_   ();
        void  write_   ();
        void  connect_ ();
        void  signal_  ();

        bool  errno_failure ();

};

}
#endif