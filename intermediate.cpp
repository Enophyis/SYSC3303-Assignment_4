//
// Created by Aaron Wang 101229105 on 04/02/25.
//
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include "Datagram.h"
#include "rcp_calls.h"

#define SERVER_PORT    6900
#define CLIENT_PORT    2300

class IntermediateServer {
private:
    DatagramSocket clientSendRcvSocket;
    DatagramSocket serverSendRcvSocket;
    in_port_t clientReplyPort;
    bool validData = false;
    std::vector<uint8_t> sharedBuffer;
    std::mutex mtx;
    std::condition_variable cv;

public:

    IntermediateServer() : clientSendRcvSocket(CLIENT_PORT), serverSendRcvSocket() {}
    void clientIntermediate()
    {
    }
    void serverIntermediate()
    {
    }
    void rcvDataClient()
    {//assuminng data is present, we rcv data and reply
            std::vector <uint8_t> buffer(100);
            DatagramPacket clientInbound(buffer, buffer.size());
            try {
                std::cout << "[CLIENT LISTENER] Waiting for client data..." << std::endl;
                clientSendRcvSocket.receive(clientInbound);
            } catch (const std::runtime_error &e) {
                std::cout << "IO Exception: likely:"
                          << "Receive Socket Timed Out." << std::endl << e.what() << std::endl;
                exit(1);
            }
            clientReplyPort = clientInbound.getPort();
            std::cout << "[CLIENT LISTENER] Got the data!" << std::endl;
            std::unique_lock lock(mtx);
            while(validData) cv.wait(lock);
            sharedBuffer = buffer;
            validData = true;
            std::cout << "[CLIENT LISTENER] Data put in buffer!" << std::endl;
            std::vector <uint8_t> replyData(100);
            DatagramPacket replyPacket(replyData, replyData.size(), InetAddress::getLocalHost(),
                                       clientReplyPort);
            clientSendRcvSocket.send(replyPacket);
            std::cout << "[CLIENT LISTENER] Letting client know data is in buffer" << std::endl;
            cv.notify_all();
    }
    void ackToClient()
    {
        //wake client up
        sendSignal(clientSendRcvSocket, clientReplyPort);

        //client asks for ack
        std::vector <uint8_t> ackMessage(100);
        DatagramPacket clientInbound(ackMessage, ackMessage.size());
        try {
            std::cout << "[CLIENT LISTENER] Waiting for client to request ack..." << std::endl;
            clientSendRcvSocket.receive(clientInbound);
        } catch (const std::runtime_error &e) {
            std::cout << "IO Exception: likely:"
                      << "Receive Socket Timed Out." << std::endl << e.what() << std::endl;
            exit(1);
        }
        std::cout << "[CLIENT LISTENER] Got ack request" << std::endl;
        //we are retrieving the ack
        std::vector <uint8_t> ack(100);
        std::unique_lock lock(mtx);
        while(!validData) cv.wait(lock);
        ack = sharedBuffer;
        validData = false;

        lock.unlock();
        cv.notify_all();
        std::cout << "[CLIENT LISTENER] replying with ack" << std::endl;
        DatagramPacket replyPacket(ack, ack.size(), InetAddress::getLocalHost(),
                                   clientInbound.getPort());
        clientSendRcvSocket.send(replyPacket);

    }



    void sendDataServer()
    {
        //wake server up
        sendSignal(serverSendRcvSocket, SERVER_PORT);

        //server now requests data
        std::vector<uint8_t> msgFromServer(100);
        DatagramPacket rcvFromServer(msgFromServer, msgFromServer.size());
        serverSendRcvSocket.receive(rcvFromServer);
        int len = rcvFromServer.getLength();
        std::cout << "[FROM SERVER] " << std::string(static_cast<const char *>(rcvFromServer.getData()), len) << std::endl;
        //should say  "gimme data"

        //now time to access the data
        std::unique_lock lock(mtx);
        while(!validData) cv.wait(lock);
        DatagramPacket outboundServer(sharedBuffer, sharedBuffer.size(), InetAddress::getLocalHost(), SERVER_PORT);
        serverSendRcvSocket.send(outboundServer);
        //cool, data has been sent

        //recieve the ack ONLY (so we can reencode it ot send to client)
        sendSignal(clientSendRcvSocket,clientReplyPort);
        std::vector<uint8_t> ackToClient(100);
        // @todo add the message here
        DatagramPacket ackPacket(ackToClient, ackToClient.size());
        serverSendRcvSocket.receive(ackPacket);

        sharedBuffer = ackToClient;

        lock.unlock();
        cv.notify_all();

        std::cout << "passing ack to client" << std::endl;

        // let server know we got the message
        std::vector<uint8_t> reply (100);
        std::string hostReply = " to client ack rcv";
        for (char c: hostReply)
        {
            reply.push_back(static_cast<uint8_t>(c));
        }
        sendMessage(serverSendRcvSocket, reply, SERVER_PORT);

    }
    // Optionally, you can wrap the client functions into a single loop:
    void runClientSide() {
        while (true) {
            rcvDataClient();
            ackToClient();
            // Optional: add a small delay if needed
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // And the server function loop:
    void runServerSide() {
        while (true) {
            sendDataServer();
            // Optional: add a small delay if needed
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
};


int main(int argc, char ** argv)
{
    IntermediateServer host;

    // Thread for client-handling side (receiving data and sending ack)
    std::thread clientThread([&host]() {
        host.runClientSide();
    });

    // Thread for server-handling side (sending data to server)
    std::thread serverThread([&host]() {
        host.runServerSide();
    });

    // Wait for both threads to finish (in practice, they run indefinitely)
    clientThread.join();
    serverThread.join();

    return 0;
}