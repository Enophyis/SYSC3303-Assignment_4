//
// Created by enophyis on 05/02/25.
//
#include <chrono>
#include <thread>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include "Datagram.h"

#define PORT 23

int main(int argc , char ** argv)
{


    std::vector<uint8_t> message(filename.begin(), filename.end());
    DatagramPacket request(message, message.size(), InetAddress::getLocalHost(), PORT);


 return 0;
}