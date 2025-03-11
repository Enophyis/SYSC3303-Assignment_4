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
#include "rcp_calls.h"

#define SERVER_PORT    6900

class Server {
public:
    Server() : sendRcvSocket(SERVER_PORT) {}

private:
    DatagramSocket sendRcvSocket;

public:
    void runServer()
    {
        std::cout << "im sleep" << std::endl;
        std::vector<uint8_t> trigger(100);
        DatagramPacket rcvPacket(trigger, trigger.size());
        sendRcvSocket.receive(rcvPacket);

        std::cout << "[SERVER] requesting data from host" << std::endl;
        std::vector<uint8_t> message(100);
        std::string request= "give data";
        for(char c: request) {
            message.push_back(c);
        }
        DatagramPacket sendPacket(message, message.size(), InetAddress::getLocalHost(), rcvPacket.getPort());
        sendRcvSocket.send(sendPacket);


        std::cout << "[SERVER] recieve data from host..." << std::endl;
        std::vector<uint8_t> rcvData(100);
        DatagramPacket rcvDataPacket(rcvData, rcvData.size());
        sendRcvSocket.receive(rcvDataPacket);

        std::cout << "[SERVER] got the data!" << std::endl;
        std::vector<uint8_t> ack(100);
        std::string response= "got data";
        for(char c: response)
        {
            message.push_back(c);
        }
        DatagramPacket ackPacket(ack, ack.size(), InetAddress::getLocalHost(), rcvPacket.getPort());
        sendRcvSocket.send(ackPacket);


        std::cout << "[SERVER] waiting for ack from host" << std::endl;
        std::vector<uint8_t> replyBuffer(100);
        DatagramPacket replyPacket(replyBuffer, replyBuffer.size());
        sendRcvSocket.receive(replyPacket);
    }

};


int main(int argc, char ** argv)
{
    Server().runServer();
}