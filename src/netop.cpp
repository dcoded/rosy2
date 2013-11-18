#include <rosy/netop.h>


namespace rosy {


netop::netop(tcp_server& server, tcp_client& client)
: state_(INIT)
, client_(&client)
, server_(&server)
{
    char buf[36];
    assert (uuid::generate (buf));
    uuid_ = std::string (buf);

    std::cout << "assigned uuid of " << uuid_ << "\n";

    server_->add_listener (this);
}


void* netop::run ()
{
    std::cout << "[OK] netop initialized\n";

    // if (client_->addr () == server_->addr ())
    //     state_ = JOINED;

    while(1)
    {
        switch(state_)
        {
            case INIT: std::cout << "INIT\n";
            {
                std::cout << "ADDRESS: " << server_->addr () << "\t" << server_->addr ().size () << "\n";
                peers_.insert (server_->addr ());
                state_ = JOINING;
            }
            break;

            case JOINING: std::cout << "JOINING\n";
            {
                state_ = JOINED;
                if (!queue_.empty ())
                {
                    lite_message msg = lite_message::unpack ( queue_.dequeue ());

                    if (msg.data () == server_->addr ())
                        state_ = JOINED;
                    else
                        state_ = JOINING;
                }
                
                client_->write (lite_message::pack ('1', uuid_, uuid::zero (), server_->addr ()));
                wait ();
            }
            break;

            case JOINED: std::cout << "JOINED\n";
            {
                state_ = READY;
                client_->write (lite_message::pack ('2', uuid_, uuid::zero (), server_->addr ()));
                wait ();
            }
            break;

            case READY: std::cout << "READY\n";
            {
                if (!queue_.empty ())
                {
                    lite_message msg = lite_message::unpack ( queue_.front ());

                    if (msg.type () == '2' && msg.from () != uuid_)
                    {
                        state_ = ADD_PEER;
                    }
                    else
                    {
                        queue_.dequeue ();
                        state_ = READY;
                    }
                }
                else wait ();
            }
            break;

            case ADD_PEER: std::cout << "ADD_PEER\n";
            {
                lite_message msg = lite_message::unpack ( queue_.front ());

                std::string endpoint = msg.data ();

                std::cout << "adding node " << endpoint << "\t" << endpoint.size () << "\n";
                std::pair<std::set<std::string>::iterator,bool> res = peers_.insert (endpoint);

                if (res.second)
                    state_ = RELINK;
                else
                    state_ = READY;              
            }
            break;

            case DEL_PEER: std::cout << "DEL_PEER\n";
            {
                lite_message msg = lite_message::unpack ( queue_.dequeue ());

                std::cout << "removing node " << msg.pack () << "\n";
                peers_.insert (msg.data ());

                state_ = READY;

                wait ();
            }
            break;

            case RELINK: std::cout << "RELINK\n";
            {
                lite_message msg = lite_message::unpack ( queue_.dequeue ());

                sleep(1);
                relink_ ();
                print_ ();
                client_->write (lite_message::pack ('2', uuid_, msg.from (), server_->addr ()));
                state_ = READY;
            }
            break;
        }
    }
    return NULL;
}

void netop::on_recv(std::string message)
{
    static int mid = 0;

    lite_message msg = lite_message::unpack (message);
    std::cout << "message(" << mid++ << "):\n"
              << "\ttype: " << msg.type () << "\n"
              << "\tfrom: " << msg.from () << "\n"
              << "\trecp: " << msg.recp () << "\n"
              << "\tdata("  << msg.size () << "):\n"
              << "\t\t" << msg.data ()
              << "\n\n";

    if (msg.recp () == uuid::zero () && msg.from () != uuid_)
    {
        client_->write (message);
        queue_.enqueue (message);
        notify();
    }
    else if (msg.recp () == uuid_)
    {
        queue_.enqueue (message);
        notify();
    }
}


void netop::relink_ ()
{
    std::set<std::string>::const_iterator it;

    it = peers_.find (server_->addr ());
    it++;

    if (it == peers_.end   ())
        it =  peers_.begin ();

    std::cout << "relink (): " << *it << "\n";
    client_->endpoint (it->c_str ());
}



void netop::print_() const
{
    std::cout << "Current Table:\n";
    std::set<std::string>::iterator it;
    int i = 0;
    for(it = peers_.begin (); it != peers_.end (); it++, i++)
    {
        std::cout << "\t[" << i << "]\t" << *it << "\t" << it->size () << "\n";
    }
}



}