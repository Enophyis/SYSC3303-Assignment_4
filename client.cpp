//
// Created by Aaron Wang 101229105 on 04/02/25.
//

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include "Datagram.h"
#include "rcp_calls.h"

#define INTERMEDIATE_PORT 2300

class Client {
public:
    Client() : sendRcvSocket() {}

private:
    DatagramSocket sendRcvSocket;

public:
    void sendData() {
    std::string filename = "message.txt";
    std::string mode = "oCTeT";
    std::vector<uint8_t> data;
    for (char c: filename)
    {
        data.push_back(static_cast<uint8_t>(c));
    }
        data.push_back(0x00);
    for (char c: mode)
    {
        data.push_back(static_cast<uint8_t>(c));
    }
        data.push_back(0x00);

        //SEND PACKET TO PORT 23
        DatagramPacket sendData(data, data.size(), InetAddress::getLocalHost(), INTERMEDIATE_PORT);
        std::cout << "To host: " << sendData.getAddressAsString() << std::endl;
        std::cout << "Destination host port: " << sendData.getPort() << std::endl;
        int len = sendData.getLength();
        std::cout << "String Data: " << std::endl;
        std::cout << std::string(static_cast<const char *>(sendData.getData()), len) << std::endl;
        std::cout << std::endl;

        try {
            sendRcvSocket.send(sendData);
        } catch (const std::runtime_error& e) {
            std::cerr << e.what();
            exit(1);
        }
        std::cout << "[CLIENT] Data sent." << std::endl;

        //===========================================================================================================
        // RCV PACKET
        std::vector<uint8_t> ack(100);
        DatagramPacket ackPacket(ack, ack.size());

        try {
            std::cout << "[CLIENT] Waiting for host ACK...\n" << std::endl;
            sendRcvSocket.receive(ackPacket);
        } catch(const std::runtime_error& e) {
            std::cerr << e.what();
            exit(1);
        }

        // Process the received datagram
        std::cout << "[CLIENT] Host ACK RCV" << std::endl;
        return;
    }

    void rcvServAck()
    {
        waitForSignal(sendRcvSocket);

        std::cout << "[CLIENT] Waiting for Server Ack" << std::endl;

        std::vector<uint8_t> request(100);
        DatagramPacket reqPacket(request, request.size(), InetAddress::getLocalHost(), INTERMEDIATE_PORT);
        sendRcvSocket.send(reqPacket);

        std::vector<uint8_t> message(100);
        DatagramPacket ackPacket(message, message.size());
        sendRcvSocket.receive(ackPacket);
        std::cout << "gothere" << std::endl;

    }
};

int main(int argc, char ** argv)
{
    Client client;
    while(true)
    {
        client.sendData();
        client.rcvServAck();
    }

}