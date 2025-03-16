#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>

#define CLIENT_PORT 9000
#define INTERMEDIATE_PORT 2300

// Helper function: Convert a byte vector to a hex string (for debugging)
std::string bytesToHex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    for (uint8_t byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte) << " ";
    }
    return oss.str();
}

// Constructs an Assignment 2–style message:
// Format: [0x00] + [opcode] + filename + [0x00] + mode + [0x00]
//
// The filename and mode are defined exactly as:
//
 //    std::string filename = "message.txt";
 //    std::string mode = "oCTeT";
//
// Then the opcode is inserted as follows:
//   - For iteration i: if i==11, use opcode 0x99 (invalid);
//   - Else if i is even, use opcode 0x01 (read);
//   - Else use opcode 0x02 (write).
std::string constructMessage(int i) {
    std::vector<uint8_t> message;
    // Start with 0 byte.
    message.push_back(0x00);

    uint8_t opcode = 0;
    if (i == 11) {
        opcode = 0x99;
        std::cout << "[Client] Iteration " << i << ": Preparing INVALID (0x99) opcode" << std::endl;
    }
    else if (i % 2 == 0) {
        opcode = 0x01;
        std::cout << "[Client] Iteration " << i << ": Preparing READ (0x01) opcode" << std::endl;
    }
    else {
        opcode = 0x02;
        std::cout << "[Client] Iteration " << i << ": Preparing WRITE (0x02) opcode" << std::endl;
    }
    message.push_back(opcode);

    // Define the filename and mode exactly as required.
    std::string filename = "message.txt";
    std::string mode = "oCTeT";

    // Append filename.
    for (char c : filename) {
        message.push_back(static_cast<uint8_t>(c));
    }
    // Append a separator 0 byte.
    message.push_back(0x00);

    // Append mode.
    for (char c : mode) {
        message.push_back(static_cast<uint8_t>(c));
    }
    // Append final 0 byte.
    message.push_back(0x00);

    std::cout << "[Client] Constructed message (hex): " << bytesToHex(message) << std::endl;
    return std::string(message.begin(), message.end());
}

class Client {
public:
    Client() : sock(CLIENT_PORT) {}

    void run() {
        // Loop for 11 iterations (i from 0 to 10)
        for (int i = 0; i < 11; i++) {
            std::cout << "\n[Client] Sending message iteration " << i << std::endl;
            std::string msg = constructMessage(i);

            // Send the message to the Intermediate host at port 2300.
            sendMessage(sock, msg, INTERMEDIATE_PORT);

            // Wait for the reply.
            std::string reply = receiveMessage(sock);
            std::cout << "[Client] Received reply (hex): ";
            std::vector<uint8_t> replyBytes(reply.begin(), reply.end());
            std::cout << bytesToHex(replyBytes) << std::endl;
        }
    }

private:
    DatagramSocket sock;
};

int main() {
    Client client;
    client.run();
    return 0;
}
