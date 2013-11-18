#include <iostream>
#include <cstdlib>

// #include <rosy/inbox.h>
// #include <rosy/outbox.h>
#include <rosy/tcp_client.h>
#include <rosy/tcp_server.h>

//#include <rosy/topology.h>
#include <rosy/ipc_broadcast.h>
#include <rosy/netop.h>

// class foo : inbox_event_listener {
// public:
//     on_recv (std::string msg)
//     {
//         switch()
//     }
// };

int main(int argc, char** argv)
{
    if(argc < 4)
    {
        std::cout << "Usage: " << argv[0] << " <server url> <remote url> <ipc url>" << std::endl;
        return -1;
    }

	//rosy::inbox inbox   (argv[1]);
    //rosy::outbox outbox (argv[2]);
    rosy::tcp_client client;
    rosy::tcp_server server;

    rosy::netop netop(server, client);


    //inbox.start ();
    //outbox.start ();
    server.start ();
    server.endpoint (argv[1]);

    while(!server.ready ());
    sleep(2);

    client.start ();
    client.endpoint (argv[2]);

    netop.start ();


    server.join ();
    return 0;
}

