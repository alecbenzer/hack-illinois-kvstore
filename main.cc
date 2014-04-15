#include "server.h"

int main(int argc, char* argv[]) {
  mm::SetDefault("test.db");
  Server test;

  test.run();
  return 0;
}
