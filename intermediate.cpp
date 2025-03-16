#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#define INTERMEDIATE_PORT 2300  // port for receiving client messages
#define SERVER_PORT       6900  // port where server listens
#define CLIENT_PORT       9000  // port to send replies back to client

class IntermediateServer {
public:
    IntermediateServer()
        : clientSocket(INTERMEDIATE_PORT), serverSocket() {}

    // start the client and server handler threads
    void run() {
        std::thread clientHandler(&IntermediateServer::handleClientSide, this);
        std::thread serverHandler(&IntermediateServer::handleServerSide, this);
        clientHandler.join();
        serverHandler.join();
    }

private:
    // socket used for client communication (bound to intermediate_port)
    DatagramSocket clientSocket;
    // ephemeral socket for server communication
    DatagramSocket serverSocket;

    std::mutex dataMutex;
    std::condition_variable dataCondVar;
    bool newDataAvailable = false;
    std::string clientData;

    // thread function to handle messages from the client
    void handleClientSide() {
        while (true) {
            std::cout << "[clienthandler] waiting for a message from the client on port "
                      << INTERMEDIATE_PORT << "..." << std::endl;
            std::string msgFromClient = receiveMessage(clientSocket);
            std::cout << "[clienthandler] received message from client." << std::endl;

            // store the message safely
            {
                std::unique_lock<std::mutex> lock(dataMutex);
                clientData = msgFromClient;
                newDataAvailable = true;
            }
            dataCondVar.notify_one();

            // send an immediate ack back to the client
            sendMessage(clientSocket, "intermediate ack: data rcvd", CLIENT_PORT);
            std::cout << "[clienthandler] sent ack to client on port " << CLIENT_PORT << "." << std::endl;
        }
    }

    // thread function to forward client data to the server and return the reply
    void handleServerSide() {
        while (true) {
            // wait until new data is available from the client
            {
                std::unique_lock<std::mutex> lock(dataMutex);
                dataCondVar.wait(lock, [this] { return newDataAvailable; });
            }
            std::cout << "[serverhandler] forwarding client data to server on port "
                      << SERVER_PORT << "..." << std::endl;
            // send the stored client message to the server
            sendMessage(serverSocket, clientData, SERVER_PORT);

            // wait for the server's reply
            std::string replyFromServer = receiveMessage(serverSocket);
            std::cout << "[serverhandler] received reply from server." << std::endl;

            // send the server's reply back to the client
            sendMessage(clientSocket, replyFromServer, CLIENT_PORT);
            std::cout << "[serverhandler] sent reply back to client on port "
                      << CLIENT_PORT << "." << std::endl;

            // mark data as processed
            {
                std::unique_lock<std::mutex> lock(dataMutex);
                newDataAvailable = false;
            }
            dataCondVar.notify_one();
        }
    }
};

int main() {
    IntermediateServer intermediate;
    intermediate.run();
    return 0;
}
