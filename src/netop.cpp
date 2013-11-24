#include <rosy/netop.h>


namespace rosy {


netop::netop(tcp_server& server, tcp_client& client)
: curr_state_(INIT)
, next_state_(INIT)
, client_(&client)
, server_(&server)
{
    char buf[36];
    assert (uuid::generate (buf));
    uuid_ = std::string (buf);

    std::cout << "assigned uuid of " << uuid_ << "\n";

    client_->add_listener (this);
    server_->add_listener (this);
}

void netop::on_timeout ()
{
    std::cout << "####################### FAULT DETECTED ############################\n";
    next_state_ = RECOVER;
    notify ();
}


void netop::advance_ ()
{
    switch(curr_state_)
    {
        case INIT     : next_state_ = JOIN;     break;
        case JOIN     : next_state_ = READY;    break;
        case ADD_PEER : next_state_ = SET_PEER; break;
        case SET_PEER : next_state_ = READY;    break;
        case DEL_PEER : next_state_ = READY;    break;
        case UNKNOWN  : next_state_ = READY;    break;
        case RECOVER  : next_state_ = READY;    break;
        case READY    :
        {
            if(queue_.empty ())
                wait ();

            else switch (message::unpack (queue_.front ()).type ())
            {
                case '1': next_state_ = ADD_PEER; break;
                case '2': next_state_ = SET_PEER; break;
                case '3': next_state_ = DEL_PEER; break;
                default : next_state_ = UNKNOWN;  break;
            }
        }
        break;
        default:
            std::cout << "advance_() reached default\n";
    }
}

void* netop::run ()
{
    std::cout << "[OK] netop initialized\n";
    while(1)
    {
        curr_state_ = next_state_;

        advance_ ();
        execute_ ();
    }
    return NULL;
}


void netop::execute_ ()
{
    switch(curr_state_)
    {
        case INIT: std::cout << "NETOP INIT\n";
        break;

        case JOIN: std::cout << "NETOP JOIN\n";
        {
            client_->write (message::pack ('1', uuid_, uuid::zero (), server_->addr ()));
            wait ();
        }
        break;

        case READY: std::cout << "NETOP READY\n";
        {
            if (queue_.empty ())
                wait ();  
        }
        break;

        case ADD_PEER: std::cout << "ADD_PEER\n";
        {
            message msg = message::unpack (queue_.front ());

            peers_.insert (msg.data ());
            relink_ ();

            if (server_->addr () < msg.data ())
            {
                std::set<std::string>::const_iterator it;
                for (it = peers_.begin (); it != peers_.end (); it++)
                    client_->write (message::pack ('2', uuid_, uuid::zero (), *it));
            }
        }
        break;

        case SET_PEER: std::cout << "SET_PEER\n";
        {
            message msg = message::unpack (queue_.dequeue ());

            peers_.insert (msg.data ());
            relink_ ();
            print_  ();
        }
        break;

        case UNKNOWN: std::cout << "UNKNOWN\n";
        {
            std::cout << queue_.dequeue () << "\n";
        }
        break;

        case RECOVER: std::cout << "RECOVER\n";
        {
            std::string fault = client_->addr ();

            std::cout << "deleting " << fault << "\t" << fault.size () << "\n";
            remove_ (fault.substr(0,15));
            relink_ ();
            print_  ();

            client_->write (message::pack ('3', uuid_, uuid::zero (), fault));
        }
        break;

        case DEL_PEER: std::cout << "DEL_PEER\n";
        {
            message msg = message::unpack (queue_.dequeue ());

            remove_ (msg.data ());
            relink_ ();
            print_  ();
        }
        break;
    }
}


void netop::on_recv(std::string message)
{
    static int mid = 0;

    rosy::message msg = message::unpack (message);
    std::cout << "message(" << mid++ << "):\n"
              << "\ttype: " << msg.type () << "\n"
              << "\tfrom: " << msg.from () << "\n"
              << "\trecp: " << msg.recp () << "\n"
              << "\tdata("  << msg.size () << "):\n"
              << "\t\t" << msg.data ()
              << "\n\n";

    if (msg.from () != uuid_)
        client_->write (message);
    
    if (msg.recp () == uuid::zero () || msg.recp () == uuid_)
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

    //std::cout << "relink (): " << *it << "\n";
    if (*it != client_->addr ())
        client_->endpoint (it->c_str ());
}


std::string netop::next_ (std::string addr)
{
    std::set<std::string>::const_iterator it;

    it = peers_.find (addr);
    it++;

    if (it == peers_.end   ())
        it =  peers_.begin ();

    return *it;
}


void netop::remove_ (std::string addr)
{
    std::set<std::string>::const_iterator it;

    it = peers_.find (addr);

    if (it != peers_.end ())
        peers_.erase (it);
}



void netop::print_ () const
{
    std::cout << "Current Table:\n";
    std::set<std::string>::iterator it;
    int i = 0;
    for (it = peers_.begin (); it != peers_.end (); it++, i++)
    {
        std::cout << "\t[" << i << "]\t" << *it << "\t" << it->size () << "\n";
    }
}



}