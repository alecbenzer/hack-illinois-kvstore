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

class Server
{
    public:
	
        //Constructor
        Server();

        //Destructor
        ~Server();
    
    private:

        //Receive from TCP Socket
        void recvCommand();

        //Parse Message
        void parse(char * message);

        //Handle Set
        char * set(char * key, char * value);

        //Handle Get
        char * get(char * key);

        //Send to TCP Socket
        void sendResponse(char * response);

        //TCP Socket
        int sock;

        //Map of Keys and Values
        std::unordered_map<std::string, std::string> kvStore;
};

#endif
