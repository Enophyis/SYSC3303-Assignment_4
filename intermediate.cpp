#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#define INTERMEDIATE_PORT 2300  // Port for receiving client messages.
#define SERVER_PORT       6900  // Port for sending messages to server.
#define CLIENT_PORT       9000  // Port to send ack/replies back to client.

class IntermediateServer {
public:
    IntermediateServer()
        : clientSock(INTERMEDIATE_PORT), serverSock() {}

    void run() {
        // Start two threads: one for client handling and one for server handling.
        std::thread tClient(&IntermediateServer::clientSide, this);
        std::thread tServer(&IntermediateServer::serverSide, this);

        tClient.join();
        tServer.join();
    }

private:
    DatagramSocket clientSock; // Bound to INTERMEDIATE_PORT for client messages.
    DatagramSocket serverSock; // Ephemeral socket used for server communications.

    std::mutex m;
    std::condition_variable cv;
    bool hasData = false;
    std::string dataFromClient;

    // Thread function to handle client-side operations.
    void clientSide() {
        while (true) {
            // Wait for a message from the client.
            std::cout << "[Intermediate-Client] Waiting for client message on port "
                      << INTERMEDIATE_PORT << "..." << std::endl;
            std::string clientMsg = receiveMessage(clientSock);
            std::cout << "[Intermediate-Client] Received from client." << std::endl;

            {
                // Store the message in a thread-safe way.
                std::unique_lock<std::mutex> lock(m);
                dataFromClient = clientMsg;
                hasData = true;
            }
            cv.notify_one();

            // Send an immediate ack to the client.
            sendMessage(clientSock, "intermediate ack: data rcvd", CLIENT_PORT);
            std::cout << "[Intermediate-Client] Sent immediate ack to client on port "
                      << CLIENT_PORT << std::endl;
        }
    }

    // Thread function to handle server-side operations.
    void serverSide() {
        while (true) {
            // Wait until clientSide has received data.
            {
                std::unique_lock<std::mutex> lock(m);
                cv.wait(lock, [this] { return hasData; });
            }
            std::cout << "[Intermediate-Server] Forwarding data to server on port "
                      << SERVER_PORT << std::endl;
            // Forward the stored client message to the server.
            sendMessage(serverSock, dataFromClient, SERVER_PORT);

            // Wait for the server’s reply (typically an ack request).
            std::string serverReply = receiveMessage(serverSock);
            std::cout << "[Intermediate-Server] Received reply from server." << std::endl;

            // Send the server's reply back to the client.
            sendMessage(clientSock, serverReply, CLIENT_PORT);
            std::cout << "[Intermediate-Server] Sent reply back to client on port "
                      << CLIENT_PORT << std::endl;

            // Mark the data as consumed.
            {
                std::unique_lock<std::mutex> lock(m);
                hasData = false;
            }
            cv.notify_one();
        }
    }
};

int main() {
    IntermediateServer is;
    is.run();
    return 0;
}
