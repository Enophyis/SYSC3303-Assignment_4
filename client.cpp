#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdint>

#define CLIENT_PORT 9000         // client's receiving port
#define INTERMEDIATE_PORT 2300   // intermediate's port for receiving client messages

// helper: convert a vector of bytes to a hex string (for debugging)
// (not used for ack display now)
std::string bytestohex(const std::vector<uint8_t>& data) {
    std::ostringstream oss;
    for (uint8_t byte : data) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(byte) << " ";
    }
    return oss.str();
}

// constructs a message using the assignment 2 format:
// format: [0x00] + [opcode] + filename + [0x00] + mode + [0x00]
// filename and mode are exactly as:
//    std::string filename = "message.txt";
//    std::string mode = "oCTeT";
// then the opcode is chosen as follows:
//   - if iteration == 11, use opcode 0x99 (invalid)
//   - else if iteration is even, use opcode 0x01 (read)
//   - else use opcode 0x02 (write)
std::string constructmessage(int iteration) {
    std::vector<uint8_t> message;
    // start with a 0 byte
    message.push_back(0x00);

    uint8_t opcode = 0;
    if (iteration == 11) {
        opcode = 0x99;
        std::cout << "[client] iteration " << iteration << ": preparing invalid (0x99) opcode." << std::endl;
    }
    else if (iteration % 2 == 0) {
        opcode = 0x01;
        std::cout << "[client] iteration " << iteration << ": preparing read (0x01) opcode." << std::endl;
    }
    else {
        opcode = 0x02;
        std::cout << "[client] iteration " << iteration << ": preparing write (0x02) opcode." << std::endl;
    }
    message.push_back(opcode);

    // define filename and mode exactly as required
    std::string filename = "message.txt";
    std::string mode = "oCTeT";

    // append filename
    for (char c : filename) {
        message.push_back(static_cast<uint8_t>(c));
    }
    // append separator 0 byte
    message.push_back(0x00);

    // append mode
    for (char c : mode) {
        message.push_back(static_cast<uint8_t>(c));
    }
    // append final 0 byte
    message.push_back(0x00);

    std::cout << "[client] constructed message (hex): " << bytestohex(message) << std::endl;
    return std::string(message.begin(), message.end());
}

class Client {
public:
    Client() : sock(CLIENT_PORT) {}

    // sends 11 messages to the intermediate and prints the reply as a string
    void run() {
        for (int i = 0; i < 11; i++) {
            std::cout << "\n[client] sending message iteration " << i << "..." << std::endl;
            std::string msg = constructmessage(i);

            // send the message to the intermediate host
            sendMessage(sock, msg, INTERMEDIATE_PORT);
            std::cout << "[client] message sent to intermediate on port " << INTERMEDIATE_PORT << "." << std::endl;

            // wait for the reply and display it as a string
            std::string reply = receiveMessage(sock);
            std::cout << "[client] received reply: " << reply << std::endl;
        }
    }

private:
    DatagramSocket sock;  // socket bound to CLIENT_PORT
};

int main() {
    Client client;
    client.run();
    return 0;
}
