#include "Datagram.h"
#include "rcp_calls.h"
#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>

#define INTERMEDIATE_PORT 2300
#define SERVER_PORT       6900
#define CLIENT_PORT       9000

class IntermediateServer {
public:
    IntermediateServer()
            : clientSock(INTERMEDIATE_PORT), serverSock() {}

    void run() {
        // two threads: one for client side, one for server side
        std::thread tClient([this]{ clientSide(); });
        std::thread tServer([this]{ serverSide(); });

        tClient.join();
        tServer.join();
    }

private:
    DatagramSocket clientSock; // bound to 2300
    DatagramSocket serverSock; // ephemeral port
    std::mutex m;
    std::condition_variable cv;

    bool hasData = false;
    std::string dataFromClient;

    // handle client steps 1,2,3,7,8
    void clientSide() {
        while (true) {
            // step 1: receive data
            std::cout << "[intermediate-client] waiting for client data\n";
            auto msg = receiveMessage(clientSock);

            if (msg == "final ack request") {
                // step 8: final ack
                std::cout << "[intermediate-client] final ack to client\n";
                sendMessage(clientSock, "final ack from intermediate", CLIENT_PORT);
                continue;
            }
            else if (msg == "forward request") {
                // step 3 done. we'll let server side handle forwarding
                std::cout << "[intermediate-client] got forward request\n";
                continue;
            }
            else {
                // assume step 1: got real data
                std::cout << "[intermediate-client] got client data: " << msg << "\n";

                {
                    std::unique_lock<std::mutex> lock(m);
                    dataFromClient = msg;
                    hasData = true;
                }
                cv.notify_all();

                // step 2: immediate ack
                std::cout << "[intermediate-client] sending ack to client\n";
                sendMessage(clientSock, "intermediate ack: data rcvd", CLIENT_PORT);
            }
        }
    }

    // handle server steps 4,5,6 (plus the actual forward from step 3)
    void serverSide() {
        while (true) {
            // wait until we have data to forward
            {
                std::unique_lock<std::mutex> lock(m);
                cv.wait(lock, [this]{ return hasData; });
            }

            // step 4: forward data to server
            std::cout << "[intermediate-server] forwarding data to server\n";
            {
                std::unique_lock<std::mutex> lock(m);
                sendMessage(serverSock, dataFromClient, SERVER_PORT);
            }

            // step 5: server wants ack
            std::cout << "[intermediate-server] waiting for server ack request\n";
            auto req = receiveMessage(serverSock);
            std::cout << "[intermediate-server] server says: " << req << "\n";

            // step 6: ack to server
            std::cout << "[intermediate-server] sending ack to server\n";
            sendMessage(serverSock, "intermediate ack to server", SERVER_PORT);

            // after step 6, we've done the forward. can mark data as consumed
            {
                std::unique_lock<std::mutex> lock(m);
                hasData = false;
            }
            cv.notify_all();
        }
    }
};

int main()
{
    IntermediateServer is;
    is.run();
    return 0;
}
