#ifndef __INCLUDE_ROSY_tcp_server_H__
#define __INCLUDE_ROSY_tcp_server_H__

#include <iostream>
#include <cstdio>
#include <cassert>
#include <cerrno>

#include <set>
#include <list>
#include <memory.h>
#include <sstream>

#include <fcntl.h>
#include <signal.h> // sigaction
#include <sys/types.h>
#include <sys/socket.h> // Needed for the socket functions
#include <netdb.h>      // Needed for the socket functions

#include <dcoady/thread.h>
#include <dcoady/circular_fifo.h>

namespace rosy {

class tcp_server_event_listener : public dcoady::thread {
public:
    virtual void on_recv(std::string) = 0;
};

struct connection {
    int       socket;
    sockaddr  address;
    socklen_t len;

    /* Thread safe FIFO queue */
    dcoady::circular_fifo<std::string, 1024> queue;
};

class tcp_server : public dcoady::thread {

    private:
        /* FSM State Variables */
        enum  state { INIT, RESET, BIND, ACCEPT, WAIT, READ, ERROR, BCAST };
        state state_;
      
        unsigned int flags_;
        enum         flags { F_RESET = 0x1, F_AGAIN = 0x2, F_SIGIN = 0x4 };

        unsigned int error_;
        enum         error { E_ADDR  = 0x1,  E_SOCK  = 0x2,
                             E_BIND  = 0x4,  E_RECV  = 0x8,
                             E_OPTN  = 0x10, E_LISN  = 0x20,
                             E_ACPT  = 0x40, E_FCTL  = 0x80 };


        /* event listeners */
        std::set<tcp_server_event_listener*> listeners_;

        std::list<connection> clients_;

        /* Socket details */
        int  socket_; // socket id


        /* config params */
        std::string host_;
        std::string port_;

        struct sigaction sig_act_;
        //struct aiocb     aio_req_;
        static std::list<tcp_server*> instances_;

    public:
        tcp_server ();

        void  add_listener (tcp_server_event_listener const* listener);
        void  endpoint (const char* endpoint);
        void* run ();

        const std::string addr () const;
        const bool ready () const;

    private:
        void reset_ ();
        void read_ ();
        void bind_ ();
        void accept_ ();
        void distribute_ (std::string message) const;
        void broadcast_ ();
        bool errno_failure_ () const;

        // bool is_http_ (const char* buffer) const;
        // void http_stats_ (std::list<connection>::iterator& client);

        static void signal_ (int sig, siginfo_t *siginfo, void *context);
};

}
#endif