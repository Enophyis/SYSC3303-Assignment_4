#ifndef RCP_CALLS_H
#define RCP_CALLS_H

#include "Datagram.h"
#include <string>
#include <vector>

// send a string msg to a given port on localhost
inline void sendMessage(DatagramSocket &sock, const std::string &msg, int port)
{
    std::vector<uint8_t> data(msg.begin(), msg.end());
    DatagramPacket packet(data, data.size(), InetAddress::getLocalHost(), port);
    sock.send(packet);
}

// receive a string msg from the socket
inline std::string receiveMessage(DatagramSocket &sock)
{
    std::vector<uint8_t> buffer(1024);
    DatagramPacket packet(buffer, buffer.size());
    sock.receive(packet);
    int len = packet.getLength();
    return std::string(reinterpret_cast<char*>(buffer.data()), len);
}

#endif // RCP_CALLS_H
