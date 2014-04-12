/**
 * @file server.cc
 * Implementation of Server
 * @date April 2014
 * @author Sai Koppula, Vikram Jayashankar
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
    listenFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);    
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
    char opcode = message[4];

    if(opcode == OP_SET)
    {
        //Read Keylength with endian fix
        uint32_t keyLength = ntohl(*((uint32_t*)&message[5]));
        
        //Get Key
        char* key = (char*)malloc(keyLength*sizeof(char) + 1);
        memcpy(key, &message[9], keyLength*sizeof(char));
        key[keyLength] = 0;
        
        //Read ValueLength with endian fix
        uint32_t valueLength = ntohl(*((uint32_t*)&message[9+keyLength]));

        //Get Value
        char* value = (char*)malloc(valueLength*sizeof(char) + 1);
        memcpy(value, &message[13+keyLength], valueLength*sizeof(char));
        value[valueLength] = 0; //null terminate the string
        
        sendResponse(set(key, value));
    }

    else if(opcode == OP_GET)
    {
        //Read Keylength
        uint32_t keyLength;
        keyLength = *( (uint32_t *) &message[5]);

        //Endian Checking
        keyLength = ntohl(keyLength);

        //Get Key
        char* key = (char*)malloc(keyLength*sizeof(char) + 1);
        memcpy(key, &message[9], keyLength*sizeof(char));
        key[keyLength] = 0;

        sendResponse(get(key));
    }
    else
    {
	    std::cout << "Invalid Opcode" << std::endl;
    }
}

char * Server::set(char * key, char * value) //might be a problem converting to std::strings when char* isn't null terminated?
{
    char* strReturn;
    int size;
    std::string strKey = key;
    std::string strValue = value;
    std::unordered_map<std::string, std::string>::const_iterator it = kvStore.find(strKey);

    kvStore[strKey] = strValue; //replace

    uint32_t msgLength = 9 + strKey.size() + strValue.size();
    uint32_t keyLtoSend = strKey.size();
    uint32_t valueLtoSend = strValue.size();

    strReturn = (char*)malloc((5 + msgLength) * sizeof(char));

    msgLength = htonl(msgLength);
    keyLtoSend = htonl(keyLtoSend);
    valueLtoSend = htonl(valueLtoSend);

    // Build strReturn
    memcpy(&strReturn[0], &msgLength, INT_LENGTH);                      // Message Length (4)
    strReturn[4] = OP_SET_ACK;                                          // OPCODE         (1)
    memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);                     // Key Length     (4)
    memcpy(&strReturn[9], &strKey, strKey.size());                      // Key            (strKey.size())
    memcpy(&strReturn[9+strKey.size()], &valueLtoSend, INT_LENGTH);     // Value Length   (4)
    memcpy(&strReturn[13+strKey.size()],&strValue,strValue.size());     // Value          (strValue.size())
    strReturn[13+strKey.size()+strValue.size()] = 0; //null terminate
    return strReturn;
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
        strReturn = (char *) malloc((13+strKey.size()) * sizeof(char) + 1);
        strReturn[4] = OP_GET_FAIL;
        keyLtoSend = htonl(keyLtoSend);
        memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
        memcpy(&strReturn[9], &strKey, strKey.size());
        uint32_t msgLength = 5 + strKey.size();
        msgLength = htonl(msgLength);
        memcpy(&strReturn[0], &msgLength, INT_LENGTH);
        strReturn[13+strKey.size()] = 0; //null terminate
    }
    else
    {
        //Construct Successful Get Message
        uint32_t keyLtoSend = strKey.size();
        uint32_t valueLtoSend;
        strValue = kvStore[key];
        valueLtoSend = strValue.size();
        strReturn = (char *) malloc((13+strKey.size()+strValue.size()) * sizeof(char) + 1);

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
        strReturn[13+strKey.size()+strValue.size()] = 0; //null terminate
    }
    return strReturn;
}

void Server::sendResponse(char * response)
{
    std::cout << response << std::endl;
}


