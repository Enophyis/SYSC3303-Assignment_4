//
// Created by enophyis on 10/03/25.
//

#include "Datagram.h"
#ifndef RCP_CALLS_H
#define RCP_CALLS_H

void waitForSignal(DatagramSocket &sendRcvSocket)
{
    std::vector<uint8_t> signalData(100);
    DatagramPacket signalPacket(signalData,signalData.size());
    sendRcvSocket.receive(signalPacket);
}
void sendSignal(DatagramSocket &sendRcvSocket, in_port_t port)
{
    std::vector<uint8_t> signal(100);
    DatagramPacket signalPacket(signal, signal.size(), InetAddress::getLocalHost(), port);
    sendRcvSocket.send(signalPacket);
}

void sendMessage(DatagramSocket &sendRcvSocket, std::vector<uint8_t> message, in_port_t port)
{
    DatagramPacket messagePacket(message, message.size(), InetAddress::getLocalHost(), port);
    sendRcvSocket.send(messagePacket);
}
std::vector<uint8_t> receiveMessage(DatagramSocket &sendRcvSocket)
{
    std::vector<uint8_t> messageData(100);
    DatagramPacket messagePacket(messageData,messageData.size());
    sendRcvSocket.receive(messagePacket);
    return messageData;
}

#endif //RCP_CALLS_H
