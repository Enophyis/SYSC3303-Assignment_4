//
// Created by Aaron Wang 101229105 on 04/02/25.
//
#include <chrono>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <stdexcept>
#include "Datagram.h"

#define PORT   69

class Server {
public:
    Server() : sendRcvSocket(PORT) {}

private:
    DatagramSocket sendRcvSocket;

public:
    void runServer() {
    while(true) {
        std::vector<uint8_t> data(100);
        DatagramPacket receivePacket(data, data.size());

        try {
            std::cout << "[SERVER] Waiting for Intermediate Packet..." << std::endl;
            sendRcvSocket.receive(receivePacket);
        } catch (const std::runtime_error &e) {
            std::cout << "IO Exception: likely:"
                      << "Receive Socket Timed Out." << std::endl << e.what() << std::endl;
            exit(1);
        }

        //Process the initial 4 byte opcode
        const uint8_t* opcodeData = static_cast<const uint8_t*>(receivePacket.getData());
        std::string opcode;
        std::vector<uint8_t> responseCode;
        bool failFlag = false;
        if (opcodeData[1] == 0x01) {
            opcode = "READ (0x01)";
            responseCode.push_back(0x03);
            responseCode.push_back(0x01);
        }
        else if (opcodeData[1] == 0x02) {
            opcode = "WRITE (0x02)";
            responseCode.push_back(0x04);
            responseCode.push_back(0x00);
        }
        else {
            opcode = "INVALID";
            failFlag = true;
        }

        // Process the received datagram.
        std::cout << "\n\n[SERVER] PACKET RECEIEVED" << std::endl;
        std::cout << "From host: " << receivePacket.getAddressAsString() << std::endl;
        std::cout << "Host port: " << receivePacket.getPort() << std::endl;
        std::cout << "Opcode: " << opcode << std::endl;
        std::cout << "String Data: " << std::endl;
        int len = receivePacket.getLength();
        std::cout << std::string(static_cast<const char *>(receivePacket.getData()), len) << std::endl; //
        std::cout << "Raw Hex Data: ";
        for (size_t i = 0; i < receivePacket.getLength(); i++) {
            uint8_t byte = static_cast<const uint8_t *>(receivePacket.getData())[i];
            // Print each byte as 2-character hexadecimal
            if (byte < 16) { // If the byte is less than 0x10, pad with a '0'
                std::cout << "0";
            }
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
        if(failFlag)
        {
            throw std::runtime_error("Runtime Exception: Invalid Opcode");
        }

        // Slow things down (wait 5 seconds)
        std::this_thread::sleep_for(std::chrono::seconds(1));


        //SEND TO INTERMEDIATE
        DatagramPacket sendPacket(responseCode, receivePacket.getLength(),
                                  receivePacket.getAddress(), receivePacket.getPort());

        std::cout << "\n\n[SERVER] SENDING PACKET:" << std::endl;
        std::cout << "To host: " << sendPacket.getAddressAsString() << std::endl;
        std::cout << "Destination host port: " << sendPacket.getPort() << std::endl;
        std::cout << "Response Code: ";
        for (size_t i = 0; i < sendPacket.getLength(); i++) {
            uint8_t byte = static_cast<const uint8_t *>(sendPacket.getData())[i];
            // Print each byte as 2-character hexadecimal
            if (byte < 16) { // If the byte is less than 0x10, pad with a '0'
                std::cout << "0";
            }
            std::cout << std::hex << static_cast<int>(byte) << " ";
        }
        std::cout << std::endl;
        DatagramSocket sendSocket;
        // Send the datagram packet to the client via the send socket.
        try {
            sendSocket.send(sendPacket);
        } catch (const std::runtime_error &e) {
            std::cerr << e.what() << std::endl;
            exit(1);
        }
        //kill the socket instance (close socket````)
        sendSocket.~DatagramSocket();

        std::cout << "\nServer: packet sent" << std::endl;
    }
   }
};


int main(int argc, char ** argv)
{
    Server().runServer();
}