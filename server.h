/**
 * @file server.h
 * Header File for Server Class
 * @date April 2014
 * @author Sai Koppula, Vikram Jayashankar
 */

#ifndef _SERVER_H
#define _SERVER_H

#include <sys/socket.h>
#include <unordered_map>
#include <map>
#include <string>
#include <algorithm>
#include <thread>
#include "mmap_allocator.h"
#include "elements.h"
#include "kvstore.pb.h"

class Server {
 public:
  bool Run(int port);

 private:
  void HandleMessage(const kvstore::Request& request, kvstore::Response* response);

  void ProcessClient(int sock);

  // Parse Message
  void parse(char* message, int fdClient);

  // Handle Set
  char* set(const char* key, String* value);

  // Handle Get
  char* get(const char* key);

  // Handle Del
  char* del(const char* key);

  // Send to TCP Socket
  void sendResponse(char* response, int fdClient);

  // TCP Socket
  int sock_;
  mm::unordered_map<std::string, std::unique_ptr<Element>> map_;
  std::vector<std::thread> threads_;
};

#endif
