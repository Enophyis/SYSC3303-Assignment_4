#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>

#define SERVER_PORT 6900
#define INTERMEDIATE_PORT 2300

class Server {
public:
    Server() : sock(SERVER_PORT) {}

    void run() {
        // step 4: wait for forwarded data
        std::cout << "[server] waiting for data\n";
        auto data = receiveMessage(sock);
        std::cout << "[server] got: " << data << "\n";

        // step 5: ask intermediate for ack
        std::cout << "[server] requesting ack\n";
        sendMessage(sock, "server ack request", INTERMEDIATE_PORT);

        // step 6: receive ack
        auto ack = receiveMessage(sock);
        std::cout << "[server] got ack: " << ack << "\n";
    }

private:
    DatagramSocket sock;
};

int main()
{
    Server server;
    server.run();
    return 0;
}
