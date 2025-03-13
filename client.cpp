#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>

#define CLIENT_PORT 9000
#define INTERMEDIATE_PORT 2300

class Client {
public:
    Client() : sock(CLIENT_PORT) {}

    void run() {
        // step 1: send data
        std::cout << "[client] sending data\n";
        sendMessage(sock, "client data payload", INTERMEDIATE_PORT);

        // step 2: immediate ack
        auto ack = receiveMessage(sock);
        std::cout << "[client] got ack: " << ack << "\n";

        // step 3: request forward
        std::cout << "[client] asking to forward\n";
        sendMessage(sock, "forward request", INTERMEDIATE_PORT);

        // step 7: final ack request
        std::cout << "[client] asking for final ack\n";
        sendMessage(sock, "final ack request", INTERMEDIATE_PORT);

        // step 8: receive final ack
        auto finalAck = receiveMessage(sock);
        std::cout << "[client] got final ack: " << finalAck << "\n";
    }

private:
    DatagramSocket sock;
};

int main()
{
    Client client;
    client.run();
    return 0;
}
