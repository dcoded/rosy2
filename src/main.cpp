#include <iostream>
#include <cstdlib>

#include <rosy/inbox.h>
#include <rosy/outbox.h>

#include <rosy/topology.h>
#include <rosy/message/publisher.h>

#include <rosy/message/message.h>

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <server url> <remote url>" << std::endl;
        return -1;
    }

	rosy::inbox inbox   (argv[1]);
    rosy::outbox outbox (argv[2]);

    rosy::topology topology(&inbox, &outbox);

    rosy::message::publisher<rosy::message::type_count> publisher(inbox);

    
    publisher.subscribe(static_cast<rosy::message::subscriber*>(&topology), rosy::message::NET_DROP);
    publisher.subscribe(static_cast<rosy::message::subscriber*>(&topology), rosy::message::NET_SET);



    inbox.start ();
    outbox.start ();
    topology.start ();
    publisher.start ();

    std::string recv;

    rosy::message::net_set join_req(argv[1], argv[2]);
    outbox.push (join_req.pack());



    inbox.join ();
    outbox.join ();

    return 0;
}

