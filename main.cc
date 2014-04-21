#include "server.h"
#include <iostream>

int main(int argc, char* argv[]) {
  mm::SetStorage("test.db");

  int port = 10000;
  if (argc >= 2) {
    port = std::stoi(argv[1]);
  }

  Server server;
  if (!server.Run(port)) {
    printf("SERVER EXITING NUB\n");
    return 1;
  }

  printf("SERVER EXITING NUB\n");
  return 0;
}
