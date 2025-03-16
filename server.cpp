#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#define SERVER_PORT 6900         // port where the server listens
#define INTERMEDIATE_PORT 2300   // port to which the server sends its reply

class Server {
public:
    Server() : sock(SERVER_PORT) {}

    // main server function: waits for a forwarded message, processes it, and sends a reply
    void run() {
        std::cout << "[server] waiting for data on port " << SERVER_PORT << "..." << std::endl;
        // wait for a message from the intermediate host
        std::string data = receiveMessage(sock);
        std::cout << "[server] received message from intermediate." << std::endl;

        // convert data to a vector of bytes for easier parsing
        std::vector<uint8_t> msg(data.begin(), data.end());
        if (msg.size() < 2) {
            std::cerr << "[server] error: message too short." << std::endl;
            return;
        }
        // the opcode is in the second byte
        uint8_t opcode = msg[1];
        std::vector<uint8_t> response;
        if (opcode == 0x01) {
            std::cout << "[server] processing read request (opcode 0x01)." << std::endl;
            // for read requests, reply with: 0x00, 0x03, 0x00, 0x01
            response = {0x00, 0x03, 0x00, 0x01};
        } else if (opcode == 0x02) {
            std::cout << "[server] processing write request (opcode 0x02)." << std::endl;
            // for write requests, reply with: 0x00, 0x04, 0x00, 0x00
            response = {0x00, 0x04, 0x00, 0x00};
        } else {
            std::cerr << "[server] invalid opcode received: " << std::hex << static_cast<int>(opcode) << std::endl;
            return;
        }

        std::cout << "[server] sending response: ";
        for (uint8_t b : response) {
            std::cout << std::hex << static_cast<int>(b) << " ";
        }
        std::cout << std::endl;

        // convert the response vector to a string
        std::string respStr(response.begin(), response.end());
        // send the response back to the intermediate host
        sendMessage(sock, respStr, INTERMEDIATE_PORT);

        // optionally, wait for a final ack
        std::string finalAck = receiveMessage(sock);
        std::cout << "[server] received final ack: " << finalAck << std::endl;
    }

private:
    DatagramSocket sock;  // socket bound to SERVER_PORT
};

int main() {
    Server server;
    server.run();
    return 0;
}
