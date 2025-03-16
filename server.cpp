#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <vector>
#include <cstdint>

#define SERVER_PORT 6900
#define INTERMEDIATE_PORT 2300

class Server {
public:
    Server() : sock(SERVER_PORT) {}

    void run() {
        // Wait for forwarded data from Intermediate.
        std::cout << "[Server] Waiting for data on port " << SERVER_PORT << std::endl;
        std::string data = receiveMessage(sock);
        std::cout << "[Server] Received message from Intermediate." << std::endl;

        // Convert the data to a byte vector for processing.
        std::vector<uint8_t> msg(data.begin(), data.end());
        if (msg.size() < 2) {
            std::cerr << "[Server] Error: message too short." << std::endl;
            return;
        }
        uint8_t opcode = msg[1];
        std::vector<uint8_t> response;
        if (opcode == 0x01) {
            std::cout << "[Server] Processing READ request." << std::endl;
            // For read requests: reply with [0x00, 0x03, 0x00, 0x01]
            response = {0x00, 0x03, 0x00, 0x01};
        } else if (opcode == 0x02) {
            std::cout << "[Server] Processing WRITE request." << std::endl;
            // For write requests: reply with [0x00, 0x04, 0x00, 0x00]
            response = {0x00, 0x04, 0x00, 0x00};
        } else {
            std::cerr << "[Server] Invalid opcode received: " << std::hex << (int)opcode << std::endl;
            return;
        }

        std::cout << "[Server] Sending response: ";
        for (uint8_t b : response) {
            std::cout << std::hex << (int)b << " ";
        }
        std::cout << std::endl;
        // Convert the response vector to a string.
        std::string respStr(response.begin(), response.end());
        // Send the response back to the Intermediate (assumed to be at INTERMEDIATE_PORT).
        sendMessage(sock, respStr, INTERMEDIATE_PORT);

        // Optionally, wait for a final acknowledgment.
        std::string finalAck = receiveMessage(sock);
        std::cout << "[Server] Received final ack: " << finalAck << std::endl;
    }

private:
    DatagramSocket sock;
};

int main() {
    Server server;
    server.run();
    return 0;
}
