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
#include <map>
#include <memory>
#include "mmap_allocator.h"
#include "elements.h"

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

#define OP_DEL 0x04
#define OP_DEL_ACK 0x14

#define BUF_SIZE 256
#define MAXBUF 256
#define PORT_NO 9999

using std::pair;
using std::string;
using std::less;
using std::map;

Server::Server()
{
  std::cout << "constructing\n";
  // Set up channel infrastructure
  struct sockaddr_in svaddr;
  memset(&svaddr, 0, sizeof(struct sockaddr_in));
  sock = socket(AF_INET, SOCK_STREAM, 0);
  svaddr.sin_family = AF_INET;

  // Endian Conversion
  std::string locHost = "127.0.0.1";
  inet_pton(AF_INET, locHost.c_str(), &svaddr.sin_addr);
  svaddr.sin_port = htons(PORT_NO);

  if (bind(sock, (struct sockaddr *)&svaddr, sizeof(struct sockaddr_in)) ==
      -1) {
    perror("bind");
  }

  /*---Make it a "listening socket"---*/
  if (listen(sock, 20) != 0) {
    perror("socket--listen");
    exit(errno);
  }

  std::cout << "done constructing\n";
}

Server::~Server() { sock = 0; }

void Server::run() {
  std::cout << "==============\n";
  std::cout << "Server Running\n";
  std::cout << "==============\n";

  /*---Forever... ---*/
  while (1) {
    int clientfd;
    struct sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);

    memset(&client_addr, 0, sizeof(struct sockaddr_in));
    client_addr.sin_family = AF_INET;

    // Endian Conversion
    std::string locHost = "127.0.0.1";
    inet_pton(AF_INET, locHost.c_str(), &client_addr.sin_addr);
    client_addr.sin_port = htons(PORT_NO);

    /*---accept a connection (creating a data pipe)---*/
    clientfd = accept(sock, (struct sockaddr *)&client_addr, &addrlen);
    // printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr),
    // ntohs(client_addr.sin_port));

    /*---Handle the Message Sent---*/
    uint32_t count;
    int r;

    r = recv(clientfd, &count, INT_LENGTH, 0);
    count = ntohl(count);
    // std::cout << "r: " << r << ", count: " << count << std::endl;
    if (r >= 0) {
      char *buf = (char *)malloc(count * sizeof(char));
      r = recv(clientfd, buf, count, 0);
      // std::cout << count << r << std::endl;
      if (r != count)
        std::cout << "Error: Invalid Read Length\n";
      else {
        parse(buf, clientfd);
      }
    }
    // send(clientfd, buffer, recv(clientfd, buffer, MAXBUF, 0), 0);

    /*---Close data connection---*/
    close(clientfd);
  }
}

void Server::parse(char *message, int fdClient) {
  char opcode = message[0];

  if (opcode == OP_SET) {
    std::cout << "Parsed to OP_SET\n";
    // Read Keylength with endian fix
    uint32_t keyLength = ntohl(*((uint32_t *)&message[1]));
    // Get Key
    char *key = (char *)malloc(keyLength * sizeof(char) + 1);
    memcpy(key, &message[5], keyLength * sizeof(char));
    key[keyLength] = 0;

    // Read ValueLength with endian fix
    uint32_t valueLength = ntohl(*((uint32_t *)&message[5 + keyLength]));

    // Get Value
    char* value = (char *)malloc(valueLength * sizeof(char) + 1);
    memcpy(value, &message[9 + keyLength], valueLength * sizeof(char));
    value[valueLength] = 0;  // null terminate the string

    E_String* val = new E_String(value);
    sendResponse(set((const char *)key, val), fdClient);

  } else if (opcode == OP_GET) {
    std::cout << "Parsed to OP_GET\n";
    // Read Keylength
    uint32_t keyLength;
    keyLength = *((uint32_t *)&message[1]);

    // Endian Checking
    keyLength = ntohl(keyLength);

    // Get Key
    char *key = (char *)malloc(keyLength * sizeof(char) + 1);
    memcpy(key, &message[5], keyLength * sizeof(char));
    key[keyLength] = 0;

    sendResponse(get((const char *)key), fdClient);
  } else if (opcode == OP_DEL) {
    std::cout << "Parsed to OP_DEL\n";
    // Read Keylength
    uint32_t keyLength;
    keyLength = *((uint32_t *)&message[1]);

    // Endian Checking
    keyLength = ntohl(keyLength);

    // Get Key
    char *key = (char *)malloc(keyLength * sizeof(char) + 1);
    memcpy(key, &message[5], keyLength * sizeof(char));
    key[keyLength] = 0;

    sendResponse(del((const char *)key), fdClient);
  } else {
    std::cout << "Invalid Opcode\n";
  }
}

char *Server::get(const char *key) {
  char *strReturn;
  int size;
  std::string strKey = key;
  E_String* value;
  std::string strValue;

  auto it = kvStore.find(strKey);

  if (it == kvStore.end()) {
    // Construct Fail Get Message
    uint32_t keyLtoSend = strKey.size();
    strReturn = (char *)malloc((9 + strKey.size()) * sizeof(char) + 1);
    strReturn[4] = OP_GET_FAIL;
    keyLtoSend = htonl(keyLtoSend);
    memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
    memcpy(&strReturn[9], key, strKey.size());
    uint32_t msgLength = 5 + strKey.size();
    msgLength = htonl(msgLength);
    memcpy(&strReturn[0], &msgLength, INT_LENGTH);
    strReturn[13 + strKey.size()] = 0;  // null terminate
  } else {
    // Construct Successful Get Message
    uint32_t keyLtoSend = strKey.size();
    uint32_t valueLtoSend;
    value = (E_String*)kvStore[key];
    strValue = value->c_str();
    valueLtoSend = strValue.size();
    strReturn = (char *)malloc(
        (13 + strKey.size() + strValue.size()) * sizeof(char) + 1);

    strReturn[4] = OP_GET_RET;
    keyLtoSend = htonl(keyLtoSend);
    valueLtoSend = htonl(valueLtoSend);
    memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
    memcpy(&strReturn[9], key, strKey.size());
    memcpy(&strReturn[9 + strKey.size()], &valueLtoSend, INT_LENGTH);
    memcpy(&strReturn[13 + strKey.size()], strValue.c_str(), strValue.size());
    uint32_t msgLength = 9 + strKey.size() + strValue.size();
    msgLength = htonl(msgLength);
    memcpy(&strReturn[0], &msgLength, INT_LENGTH);
    strReturn[13 + strKey.size() + strValue.size()] = 0;  // null terminate
  }

  return strReturn;
}

char *Server::del(const char *key) {
  char *strReturn;
  int size;
  std::string strKey = key;
  
  auto it = kvStore.find(strKey);

  if (it != kvStore.end()) {
    kvStore.erase(strKey);
  }

  uint32_t keyLtoSend = strKey.size();
  strReturn = (char *)malloc((9 + strKey.size()) * sizeof(char) + 1);
  strReturn[4] = OP_DEL_ACK;
  keyLtoSend = htonl(keyLtoSend);
  memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
  memcpy(&strReturn[9], key, strKey.size());
  uint32_t msgLength = 5 + strKey.size();
  msgLength = htonl(msgLength);
  memcpy(&strReturn[0], &msgLength, INT_LENGTH);
  strReturn[13 + strKey.size()] = 0; 

  return strReturn;
}

char *Server::set(const char *key, E_String* value)  // might be a problem converting to
                                      // std::strings when char* isn't null
                                      // terminated?
{
  char *strReturn;
  int size;
  std::string strKey = std::string(key);
  
  
  auto it = kvStore.find(strKey);

  kvStore[strKey] = value;  // replace

  uint32_t msgLength = 9 + strKey.size() + value->size();
  uint32_t keyLtoSend = strKey.size();
  uint32_t valueLtoSend = value->size();

  strReturn = (char *)malloc((4 + msgLength) * sizeof(char));

  msgLength = htonl(msgLength);
  keyLtoSend = htonl(keyLtoSend);
  valueLtoSend = htonl(valueLtoSend);
  // Build strReturn
  memcpy(&strReturn[0], &msgLength, INT_LENGTH);
  strReturn[4] = OP_SET_ACK;                     
  memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH); 
  memcpy(&strReturn[9], key, strKey.size());       
  memcpy(&strReturn[9 + strKey.size()], &valueLtoSend,
         INT_LENGTH);  
  memcpy(&strReturn[13 + strKey.size()], value->c_str(),
         value->size());

  return strReturn;
}

void Server::sendResponse(char *response, int fdClient) {
  uint32_t msgLength = ntohl(*((uint32_t *)&response[0]));
  send(fdClient, response, msgLength + 4, 0);
}
