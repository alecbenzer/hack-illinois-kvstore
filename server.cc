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
#include <cstring>
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

using std::pair;
using std::string;
using std::less;
using std::map;

bool Server::Run(int port) {
  sock_ = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_ == -1) {
    perror("failed creating socket");
    return false;
  }

  struct sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(port);

  if (bind(sock_, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("failed binding socket");
    return false;
  }

  printf("addr.sin_port: %d\n", addr.sin_port);

  if (listen(sock_, 20) != 0) {
    perror("failed listening on socket");
    return false;
  }

  printf("Listening on port %d\n", ntohs(addr.sin_port));

  while (1) {
    printf("Waiting...\n");
    int clientfd = accept(sock_, NULL, NULL);
    printf("got one: %d\n", clientfd);

    threads_.push_back(std::thread(&Server::ProcessClient, this, clientfd));
  }
}

void Server::ProcessClient(int sock) {
  printf("ProcessClient(%d)\n", sock);
  fflush(stdout);

  while (1) {
    uint32_t bytes;
    if (read(sock, &bytes, sizeof(bytes)) < 0) {
      perror("Error reading from socket");
      return;
    }
    bytes = ntohl(bytes);
    char* buffer = static_cast<char*>(malloc(bytes + 1));
    if (read(sock, buffer, bytes) != bytes) {
      fprintf(stderr, "Error reading from socket: expecting %zd bytes\n", bytes);
      return;
    }
    buffer[bytes] = '\0';

    kvstore::Request request;
    kvstore::Response response;
    if (!request.ParseFromString(buffer)) {
      fprintf(stderr, "Error parsing protobuf message\n");
      return;
    }

    HandleMessage(request, &response);

    std::string payload;
    if (!response.SerializeToString(&payload)) {
      fprintf(stderr, "can't serialize dat shit\n");
    }

    bytes = htonl(payload.size());
    printf("Sending size\n");
    write(sock, static_cast<void*>(&bytes), sizeof(bytes));
    printf("Sending payload '%s'\n", payload.c_str());
    write(sock, payload.c_str(), payload.size());
  }
  close(sock);
}

void Server::HandleMessage(const kvstore::Request& request, kvstore::Response* response) {
  if (request.has_get_request()) {
    const std::string& key = request.get_request().key();
    Element* element = map_[key].get();
    if (element == NULL) {
      response->set_error("Key doesn't exist");
      return;
    }
    if (element->type() != Element::STRING) {
      response->set_error("Value not a string");
      return;
    }
    String* value = static_cast<String*>(element);

    response->mutable_get_response()->set_key(key);
    response->mutable_get_response()->set_value(value->str().c_str());
  } else {
    response->set_error("Unknown request format");
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
    char *value = (char *)malloc(valueLength * sizeof(char) + 1);
    memcpy(value, &message[9 + keyLength], valueLength * sizeof(char));
    value[valueLength] = 0;  // null terminate the string

    String *val = new String(value);
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
  String *value;
  std::string strValue;

  auto it = map_.find(strKey);

  if (it == map_.end()) {
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
    value = (String *)map_[key].get();
    strValue = value->str().c_str();
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

  auto it = map_.find(strKey);

  if (it != map_.end()) {
    map_.erase(strKey);
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

char *Server::set(const char *key,
                  String *value)  // might be a problem converting to
                                    // std::strings when char* isn't null
                                    // terminated?
{
  char *strReturn;
  int size;
  std::string strKey(key);

  map_[strKey] = std::unique_ptr<Element>(value);  // replace

  uint32_t msgLength = 9 + strKey.size() + value->str().size();
  uint32_t keyLtoSend = strKey.size();
  uint32_t valueLtoSend = value->str().size();

  strReturn = (char *)malloc((4 + msgLength) * sizeof(char));

  msgLength = htonl(msgLength);
  keyLtoSend = htonl(keyLtoSend);
  valueLtoSend = htonl(valueLtoSend);
  // Build strReturn
  memcpy(&strReturn[0], &msgLength, INT_LENGTH);
  strReturn[4] = OP_SET_ACK;
  memcpy(&strReturn[5], &keyLtoSend, INT_LENGTH);
  memcpy(&strReturn[9], key, strKey.size());
  memcpy(&strReturn[9 + strKey.size()], &valueLtoSend, INT_LENGTH);
  memcpy(&strReturn[13 + strKey.size()], value->str().c_str(), value->str().size());

  return strReturn;
}

void Server::sendResponse(char *response, int fdClient) {
  uint32_t msgLength = ntohl(*((uint32_t *)&response[0]));
  send(fdClient, response, msgLength + 4, 0);
}
