/**
 * @file server.cc
 * Implementation of Server
 * @date April 2014
 * @author Sai Koppula
 */

#include "server.h"
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <cstdint>

#define INT_LENGTH 4
#define OP_SET	    0x02
#define OP_SET_ACK  0x12
#define OP_GET	    0x03
#define OP_GET_RET  0x13
#define OP_GET_FAIL 0x23

Server::Server()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
}

Server::~Server()
{
    sock = 0;
}

void Server::recvCommand()
{
    uint32_t count;
    int r;

    r = read(sock, &count, INT_LENGTH);
    count = ntohl(count);
    if(r >= 0)
    {
	char * buf = (char *) malloc(count*sizeof(char));
	r = read(sock, buf, count);
	if(r != count) std::cout << "Error: Invalid Read Length." << std::endl;

	parse(buf);
    }
}

void Server::parse(char * message)
{
    char opcode = message[0];

    if(opcode == OP_SET)
    {
	
    }
    else if(opcode == OP_GET)
    {
	//Read Keylength
	uint32_t keyLength;
	keyLength = *( (uint32_t *) &message[1]);

	//Endian Checking
	keyLength = ntohl(keyLength);

	//Get Key
	char * key = (char *) malloc(keyLength*sizeof(char));
	memcpy(key, &message[2], keyLength*sizeof(char));

	sendResponse(get(key));
    }
    else
    {
	std::cout << "Invalid Opcode." << std::endl;
    }
}

char * Server::set(char * key, char * value)
{
    return NULL;
}

char * Server::get(char * key)
{
    char * strReturn;
    int size;
    std::string strKey = key;
    std::string strValue;
    std::unordered_map<std::string, std::string>::const_iterator it = kvStore.find(strKey);

    if(it == kvStore.end())
    {
	//Construct Fail Get Message
	uint32_t keyLtoSend = strKey.size();
	strReturn = (char *) malloc((9+strKey.size()) * sizeof(char));
	strReturn[4] = OP_GET_FAIL;
	keyLtoSend = htonl(keyLtoSend);
	memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
	memcpy(&strReturn[9], &strKey, strKey.size());
	uint32_t msgLength = 5 + strKey.size();
	msgLength = htonl(msgLength);
	memcpy(&strReturn[0], &msgLength, INT_LENGTH);
	return strReturn;
    }

    else
    {
	//Construct Successful Get Message
	uint32_t keyLtoSend = strKey.size();
	uint32_t valueLtoSend;
	strValue = kvStore[key];
	valueLtoSend = strValue.size();
	strReturn = (char *) malloc((9+strKey.size()+strValue.size()) * sizeof(char));
	strReturn[4] = OP_GET_RET;
	keyLtoSend = htonl(keyLtoSend);
	valueLtoSend = htonl(valueLtoSend);
	memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
	memcpy(&strReturn[9], &strKey, strKey.size());
	memcpy(&strReturn[9+strKey.size()], &valueLtoSend, INT_LENGTH);
	memcpy(&strReturn[13+strKey.size()], &strValue, strValue.size());
	uint32_t msgLength = 9 + strKey.size() + strValue.size();
	msgLength = htonl(msgLength);
	memcpy(&strReturn[0], &msgLength, INT_LENGTH);
	return strReturn;
    }


   
}

void Server::sendResponse(char * response)
{
    return;
}


