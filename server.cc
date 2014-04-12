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
#include <string>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define INT_LENGTH 4
#define OP_SET 0x02
#define OP_SET_ACK 0x12
#define OP_GET 0x03
#define OP_GET_RET 0x13
#define OP_GET_FAIL 0x23

#define BUF_SIZE    256
#define MAXBUF	    256
#define PORT_NO	    9999

Server::Server()
{
    //Set up channel infrastructure
    struct  sockaddr_in svaddr;
    memset(&svaddr, 0, sizeof(struct sockaddr_in));
    sock = socket(AF_INET, SOCK_STREAM, 0);
    svaddr.sin_family = AF_INET;

    //Endian Conversion
    std::string locHost = "127.0.0.1";
    inet_pton(AF_INET, locHost.c_str(), &svaddr.sin_addr);
    svaddr.sin_port = htons(PORT_NO);

    if(bind(sock, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in)) == -1) 
    {
	perror("bind");
    }

    //fcntl(sock, F_SETFL, O_NONBLOCK);

    /*---Make it a "listening socket"---*/
    if ( listen(sock, 20) != 0 )
    {
	perror("socket--listen");
	exit(errno);
    }


}

Server::~Server() 
{ 
    sock = 0; 
}

void Server::main_loop()
{

    char buffer[BUF_SIZE];
    /*---Forever... ---*/
    while (1)
    {	
    int clientfd;
    struct sockaddr_in client_addr;
    socklen_t addrlen=sizeof(client_addr);

    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    client_addr.sin_family = AF_INET;

    //Endian Conversion
    std::string locHost = "127.0.0.1";
    inet_pton(AF_INET, locHost.c_str(), &client_addr.sin_addr);
    client_addr.sin_port = htons(PORT_NO);

    /*---accept a connection (creating a data pipe)---*/
    clientfd = accept(sock, (struct sockaddr*)&client_addr, &addrlen);
    //printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    /*---Handle the Message Sent---*/
	uint32_t count;
	int r;
	
	r = recv(clientfd, &count, INT_LENGTH, 0);
	count = ntohl(count);
	if(r >= 0)
	{
		char * buf = (char *) malloc(count*sizeof(char));
		r = recv(sock, buf, count, 0);
		if(r != count) std::cout << "Error: Invalid Read Length." << std::endl;
		else parse(buf, clientfd);
	}
    //send(clientfd, buffer, recv(clientfd, buffer, MAXBUF, 0), 0);

    /*---Close data connection---*/
    close(clientfd);
    }
    

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
		else parse(buf,0);
	}
}

void Server::parse(char * message, int fdClient)
{
    char opcode = message[0];

    if (opcode == OP_SET) {
      // Read Keylength with endian fix
      uint32_t keyLength = ntohl(*((uint32_t *)&message[5]));

      // Get Key
      char *key = (char *)malloc(keyLength * sizeof(char) + 1);
      memcpy(key, &message[5], keyLength * sizeof(char));
      key[keyLength] = 0;

      // Read ValueLength with endian fix
      uint32_t valueLength = ntohl(*((uint32_t *)&message[9 + keyLength]));

      // Get Value
      char *value = (char *)malloc(valueLength * sizeof(char) + 1);
      memcpy(value, &message[9 + keyLength], valueLength * sizeof(char));
      value[valueLength] = 0;  // null terminate the string

      sendResponse(set(key, value, fdClient), fdClient);
    } else if (opcode == OP_GET) {
      // Read Keylength
      uint32_t keyLength;
      keyLength = *((uint32_t *)&message[1]);

      // Endian Checking
      keyLength = ntohl(keyLength);

      // Get Key
      char *key = (char *)malloc(keyLength * sizeof(char) + 1);
      memcpy(key, &message[5], keyLength * sizeof(char));
      key[keyLength] = 0;

      sendResponse(get(key, fdClient), fdClient);
    } else {
      std::cout << "Invalid Opcode" << std::endl;
    }
}

char * Server::get(char * key, int fdClient) {
  char *strReturn;
  int size;
  std::string strKey = key;
  std::string strValue;
  std::unordered_map<std::string, std::string>::const_iterator it =
	  kvStore.find(strKey);

  if (it == kvStore.end()) {
	// Construct Fail Get Message
	uint32_t keyLtoSend = strKey.size();
	strReturn = (char *)malloc((9 + strKey.size()) * sizeof(char) + 1);
	strReturn[4] = OP_GET_FAIL;
	keyLtoSend = htonl(keyLtoSend);
	memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
	memcpy(&strReturn[9], &strKey, strKey.size());
	uint32_t msgLength = 5 + strKey.size();
	msgLength = htonl(msgLength);
	memcpy(&strReturn[0], &msgLength, INT_LENGTH);
	strReturn[13 + strKey.size()] = 0;  // null terminate
  } else {
	// Construct Successful Get Message
	uint32_t keyLtoSend = strKey.size();
	uint32_t valueLtoSend;
	strValue = kvStore[key];
	valueLtoSend = strValue.size();
	strReturn = (char *)malloc(
		(13 + strKey.size() + strValue.size()) * sizeof(char) + 1);

	strReturn[4] = OP_GET_RET;
	keyLtoSend = htonl(keyLtoSend);
	valueLtoSend = htonl(valueLtoSend);
	memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
	memcpy(&strReturn[9], &strKey, strKey.size());
	memcpy(&strReturn[9 + strKey.size()], &valueLtoSend, INT_LENGTH);
	memcpy(&strReturn[13 + strKey.size()], &strValue, strValue.size());
	uint32_t msgLength = 9 + strKey.size() + strValue.size();
	msgLength = htonl(msgLength);
	memcpy(&strReturn[0], &msgLength, INT_LENGTH);
	strReturn[13 + strKey.size() + strValue.size()] = 0;  // null terminate
  }
  return strReturn;
}

char *Server::set(char * key, char * value, int fdClient)		// might be a problem converting to
												// std::strings when char* isn't null
												// terminated?
{
  char *strReturn;
  int size;
  std::string strKey = key;
  std::string strValue = value;
  std::unordered_map<std::string, std::string>::const_iterator it =
	  kvStore.find(strKey);

  kvStore[strKey] = strValue;  // replace

  uint32_t msgLength = 9 + strKey.size() + strValue.size();
  uint32_t keyLtoSend = strKey.size();
  uint32_t valueLtoSend = strValue.size();

  strReturn = (char *)malloc((4 + msgLength) * sizeof(char));

  msgLength = htonl(msgLength);
  keyLtoSend = htonl(keyLtoSend);
  valueLtoSend = htonl(valueLtoSend);

  // Build strReturn
  memcpy(&strReturn[0], &msgLength, INT_LENGTH);   // Message Length (4)
  strReturn[4] = OP_SET_ACK;                       // OPCODE         (1)
  memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);  // Key Length     (4)
  memcpy(&strReturn[9], &strKey, strKey.size());   // Key (strKey.size())
  memcpy(&strReturn[9 + strKey.size()], &valueLtoSend,
		 INT_LENGTH);  // Value Length   (4)
  memcpy(&strReturn[13 + strKey.size()], &strValue,
		 strValue.size());  // Value          (strValue.size())

  return strReturn;
}

void Server::sendResponse(char * response, int fdClient) {
	
	std::cout << response << std::endl;
	
	uint32_t msgLength = ntohl(*((uint32_t *)&response[0]));
	send(fdClient, response, msgLength, 0);
	
	
}




/*
 int32_t size = -1;
 int check, c;
 char buf[BUF_SIZE];
 
 std::cout << "In main loop!" << std::endl;
 while(size < 0)
 {
 //std::cout << size << std::endl;
 size = recvfrom(sock, buf, BUF_SIZE, 0, NULL, NULL);
 }
 
 std::cout << "Message Received: " << buf << std::endl;
 
 std::string strAck = "Got it Vikram!";
 
 struct sockaddr_in raddr;
 memset(&raddr, 0, sizeof(struct sockaddr_in));
 
 raddr.sin_family = AF_INET;
 
 //convert the ip-address to human readable form
 std::string locHost = "127.0.0.1";
 inet_pton(AF_INET, locHost.c_str() , &raddr.sin_addr);
 raddr.sin_port = htons(PORT_NO);
 
 std::cout << "Message about to send." << std::endl;
 //send the message to its destination
 c = sendto(sock, strAck.c_str(), strAck.size(), 0, (struct sockaddr*)&raddr, sizeof(raddr));
 
 if (c != sizeof(strAck))
 {
 std::cout << "Bad write " << c << " != " << sizeof(strAck) << std::endl;
 }
 */
