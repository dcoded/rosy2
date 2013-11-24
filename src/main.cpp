#include <iostream>
#include <cstdlib>

#include <rosy/tcp_client.h>
#include <rosy/tcp_server.h>

#include <rosy/netop.h>

int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <server url> <remote url>" << std::endl;
        return -1;
    }

    rosy::tcp_client client;
    rosy::tcp_server server;
    rosy::netop netop(server, client);

    server.endpoint (argv[1]);
    server.start ();

    while (!server.ready ()) ;

    client.endpoint (argv[2]);
    client.start ();

    netop.start ();
    server.join ();

    /* Should never be reached ... */
    return 0;
}

