/**
 * @file server.h
 * Header File for Server Class
 * @date April 2014
 * @author Sai Koppula
 */

#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <unordered_map>
#include <string>

class Server {
public:
    // Constructor
    Server();

    //Destructor
    ~Server();

    //Main Loop for testing
    void main_loop();

private:
    // Receive from TCP Socket
    void recvCommand();

    // Parse Message
    void parse(char* message, int fdClient);

    // Handle Set
    char* set(const char* key, const char* value, int fdClient);

    // Handle Get
    char* get(const char* key, int fdClient);

    // Send to TCP Socket
    void sendResponse(char* response, int fdClient);

    // TCP Socket
    int sock;

    // Map of Keys and Values
    std::unordered_map<std::string, std::string> kvStore;

};

#endif
