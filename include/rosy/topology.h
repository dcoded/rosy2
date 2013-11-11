#ifndef __INCLUDE_ROSY_TOPOLOGY_H__
#define __INCLUDE_ROSY_TOPOLOGY_H__

#include <map>
#include <string>

#include <msgpack.hpp>

#include <dcoady/thread.h>

#include <rosy/message/subscriber.h>
#include <rosy/message/message.h>


namespace rosy {

class topology : public dcoady::thread,
                 public message::subscriber {

    private:
        std::map<std::string, std::string> endpoints_;
        inbox* inbox_;
        outbox* outbox_;

    public:
        topology (inbox* inbox, outbox* outbox)
        : inbox_(inbox)
        , outbox_(outbox)
        {
            //endpoints_[inbox_->addr ()] = outbox->addr ();
        }

        void* run()
        {
            while(1)
            {
                std::cout << "topo::loop()\n";
                if(outbox_->timeout())
                {
                    std::cout << outbox_->addr() << " crashed\n";
                    std::cout << "switched to " << endpoints_[outbox_->addr()] << "\n";
                    recover (endpoints_[outbox_->addr()]);
                }
                print_table();
                sleep(1);
            }
            return NULL;
        }

        void recover(std::string new_endpoint)
        {
            std::cout << "send drop(" << outbox_->addr() << ") origin = " << inbox_->addr() << ")\n";
            outbox_->push (message::net_drop(inbox_->addr (), outbox_->addr (), new_endpoint).pack ());
            outbox_->update_endpoint (new_endpoint);
            while (outbox_->connect() == false)
            {
                std::cout << "connecting...\n";
            }

            std::cout << "reset timeout\n";
            outbox_->timeout(true);
        }

        void on_message(message::type type, std::string msg)
        {
            switch(type)
            {
                // case message::NET_ADD:
                // {
                //     message::net_add message;
                //     message.unpack(msg);

                //     add_endpoint(message);
                // }
                // break;

                case message::NET_SET:
                {
                    message::net_set message;
                    message.unpack(msg);

                    set_endpoint(message);
                }
                break;

                case message::NET_DROP:
                {
                    message::net_drop message;
                    message.unpack(msg);

                    drop_endpoint(message);
                }
                break;

                default:
                    std::cout << "unknown packet\n";
            }
        }

        void set_endpoint (message::net_set& message)
        {
            std::string origin = message.origin ();
            std::string to     = message.to ();
            std::map<std::string, std::string>& table = message.table();


            if (origin != inbox_->addr () && to == outbox_->addr ()) // expand ring
            {
                std::cout << "updating outbox to " << origin << "\n";
                outbox_->update_endpoint (origin);
                while (outbox_->connect() == false) ;

                outbox_->push (message::net_set(inbox_->addr (), outbox_->addr ()).pack ());
            }

            endpoints_[origin]     = to;
            table[inbox_->addr ()] = outbox_->addr ();


            if (origin != inbox_->addr ())
            {
                std::cout << "sending to " << outbox_->addr() << "\n";
                outbox_->push (message.pack ());
            }
            else endpoints_ = table;
        }

        void drop_endpoint(message::net_drop& message)
        {
            std::string origin = message.origin ();
            std::string bad = message.bad ();
            std::string next = message.next ();

            std::cout << "NET_DROP: " << origin << " drop(" << bad << ")\n";

            std::map<std::string, std::string>::iterator it;

            std::cout << "changing " << bad << " to " << endpoints_[bad] << "\n";
            for(it = endpoints_.begin (); it != endpoints_.end (); it++)
            {
                if(it->second == bad)
                    it->second = next;
            }

            endpoints_.erase(bad);

            if (origin != inbox_->addr ())
            {
                outbox_->push (message.pack ());
            }
        }


        void print_table()
        {
            std::cout << "Current Table:\n";
            std::map<std::string, std::string>::iterator it;
            for(it = endpoints_.begin (); it != endpoints_.end (); it++)
            {
                std::cout << it->first << " -> " << it->second << "\n";
            }
        }

};

}




#endif