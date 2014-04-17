#include "server.h"
#include <iostream>

int main(int argc, char* argv[]) {
  std::cout << "about to set the default file.\n";
  mm::SetStorage("test.db");
  std::cout << "successfully set default\n";
  Server test;
  std::cout << "declared server\n";
  test.run();
  return 0;
}
