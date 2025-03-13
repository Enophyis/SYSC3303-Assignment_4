SYSC 3303 Assignment 4

Initialization Order

//this assumes you have g++ installed for compilation

$ g++ server.cpp -o server			//Start the Server – Binds to its port and waits for forwarded data.
$ g++ client.cpp -o client			//Start the Intermediate – Listens for messages from both the Client and the Server.
$ g++ intermediate.cpp -o intermediate		//Start the Client – Sends data to the Intermediate and ultimately receives the final acknowledgment.

//to start the programs. (order is essential)
$ ./server
$ ./intermediate
$ ./client
    

Files:
    client.cpp: Initiates communication by sending data to the Intermediate and later requests/receives a final acknowledgment.
    intermediate.cpp: Acts as the go‐between, forwarding data from the Client to the Server and handling acknowledgment flows in both directions.
    server.cpp: Receives forwarded data from the Intermediate, requests an acknowledgment, and completes the communication loop.
    rcp_calls.h: Provides helper functions for sending and receiving string messages using UDP sockets and packets.

Questions & Answers

1) We use more than one thread in intermediate because intermediate needs to handle simultaneous communication from two endpoints (Client and Server), multiple threads allow it to listen and respond to each side without blocking the other, ensuring efficient concurrency.

1)It's necessary to use synchronization in the Intermediate task because if the Intermediate uses multiple threads, shared data structures or resources (e.g., buffers) must be protected with synchronization to prevent race conditions and ensure data integrity. If strictly sequential logic is used (e.g., blocking one side at a time), synchronization might be less critical, but typically concurrency requires it.
