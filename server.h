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
#include <map>
#include <string>
#include <algorithm>
#include "mmap_allocator.h"

class Server {
 public:
  // Constructor
  Server();

  // Destructor
  ~Server();

    //Main Loop for testing
    void run();


private:
    // Parse Message
    void parse(char* message, int fdClient);

    // Handle Set
    char* set(const char* key, const char* value);

    // Handle Get
    char* get(const char* key);

    // Handle Del
    char* del(const char* key);

    // Send to TCP Socket
    void sendResponse(char* response, int fdClient);

    // TCP Socket
    int sock;

    std::unique_ptr<MMapAllocator<std::pair<const std::string, std::string>>> alloc;

    // Map of Keys and Values
    std::map<std::string, std::string, std::less<std::string>, MMapAllocator<std::pair<const std::string, std::string>>> kvStore;

  // Map of Keys and Values
  std::unordered_map<std::string, std::string> kvStore;
};

#endif
