//
// Created by Aaron Wang 101229105 on 04/02/25.
//
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include "Datagram.h"

#define PORT    23
#define SERVPORT 69

class IntermediateServer {
public:
    IntermediateServer() : clientSendRcvSocket(PORT), serverSendRcvSocket() {}

private:
    DatagramSocket clientSendRcvSocket;
    DatagramSocket serverSendRcvSocket;

public:
    void runIntermediateHost() {
        while (true) {
            //================== RCV PACKET FROM CLIENT FROM PORT 23 ==================
            std::vector<uint8_t> clientdata(100);
            DatagramPacket clientInbound(clientdata, clientdata.size());
            //Await for client packet
            try {
                std::cout << "[INTERMEDIATE] Waiting for client packet..." << std::endl;
                clientSendRcvSocket.receive(clientInbound);
            } catch (const std::runtime_error &e) {
                std::cout << "IO Exception: likely:"
                          << "Receive Socket Timed Out." << std::endl << e.what() << std::endl;
                exit(1);
            }

            // Process the received datagram from client
            std::cout << "\n\n[INTERMEDIATE] PACKET RECEIEVED FROM CLIENT"<< std::endl;
            std::cout << "From host: " << clientInbound.getAddressAsString() << std::endl;
            std::cout << "Host port: " << clientInbound.getPort() << std::endl;
            std::cout << "String Data: " << std::endl;
            int len = clientInbound.getLength();
            std::cout << std::string(static_cast<const char *>(clientInbound.getData()), len) << std::endl;
            std::cout << "Raw Hex Data: ";
            for (size_t i = 0; i < clientInbound.getLength(); i++) {
                uint8_t byte = static_cast<const uint8_t *>(clientInbound.getData())[i];
                if (byte < 16) {
                    std::cout << "0";
                }
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::endl;

            //================== SEND PACKET TO SERVER ON PORT 69 ==================
            //prepare to send packet to server
            DatagramPacket outboundToServer(clientdata, clientInbound.getLength(),
                                      clientInbound.getAddress(), SERVPORT);

            std::cout << "\n\n[INTERMEDIATE] SENDING PACKET TO SERVER" << std::endl;
            std::cout << "To host: " << outboundToServer.getAddressAsString() << std::endl;
            std::cout << "Destination host port: " << outboundToServer.getPort() << std::endl;
            std::cout << "String Data: " << std::endl;
            std::cout << std::string(static_cast<const char *>(outboundToServer.getData()), len) << std::endl; //
            std::cout << "Raw Hex Data: ";
            for (size_t i = 0; i < outboundToServer.getLength(); i++) {
                uint8_t byte = static_cast<const uint8_t *>(outboundToServer.getData())[i];
                // Print each byte as 2-character hexadecimal
                if (byte < 16) { // If the byte is less than 0x10, pad with a '0'
                    std::cout << "0";
                }
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::endl;

            try {
                serverSendRcvSocket.send(outboundToServer);
            } catch (const std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
                exit(1);
            }
            std::cout << "Packet sent" << std::endl;

            //================== RCV RESPONSE PACKET FROM SERVER ==================
            std::vector<uint8_t> serverdata(100);
            DatagramPacket serverInbound(serverdata, serverdata.size());
            //await response packet from server
            try {
                std::cout << "[INTERMEDIATE] Waiting for server packet..." << std::endl;
                serverSendRcvSocket.receive(serverInbound);
            } catch (const std::runtime_error &e) {
                std::cout << "IO Exception: likely:"
                          << "Receive Socket Timed Out." << std::endl << e.what() << std::endl;
                exit(1);
            }

            // Process the received datagram from server.
            std::cout << "\n\n[INTERMEDIATE] PACKET RECIEVED FROM SERVER" << std::endl;
            std::cout << "From host: " << serverInbound.getAddressAsString() << std::endl;
            std::cout << "Host port: " << serverInbound.getPort() << std::endl;
            std::cout << "Response Code: ";
            for (size_t i = 0; i < serverInbound.getLength(); i++) {
                uint8_t byte = static_cast<const uint8_t *>(serverInbound.getData())[i];
                // Print each byte as 2-character hexadecimal
                if (byte < 16) { // If the byte is less than 0x10, pad with a '0'
                    std::cout << "0";
                }
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::endl;

            //================== SEND PACKET TO CLIENT ==================
            DatagramPacket clientSendPacket(serverdata, serverInbound.getLength(),
                                            serverInbound.getAddress(), clientInbound.getPort());

            std::cout << "\n\n[INTERMEDIATE] SENDING PACKET TO CLIENT" << std::endl;
            std::cout << "To host: " << clientSendPacket.getAddressAsString() << std::endl;
            std::cout << "Destination host port: " << clientSendPacket.getPort() << std::endl;
            std::cout << "Response Code: ";
            for (size_t i = 0; i < clientSendPacket.getLength(); i++) {
                uint8_t byte = static_cast<const uint8_t *>(clientSendPacket.getData())[i];
                // Print each byte as 2-character hexadecimal
                if (byte < 16) { // If the byte is less than 0x10, pad with a '0'
                    std::cout << "0";
                }
                std::cout << std::hex << static_cast<int>(byte) << " ";
            }
            std::cout << std::endl;

            try {
                clientSendRcvSocket.send(clientSendPacket);
            } catch (const std::runtime_error &e) {
                std::cerr << e.what() << std::endl;
                exit(1);
            }
            std::cout << "[INTERMEDIATE] Packet sent" << std::endl;
        }
    }
};


int main(int argc, char ** argv)
{
    IntermediateServer().runIntermediateHost();
}