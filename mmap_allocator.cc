#include "mmap_allocator.h"

namespace mm {

int default_fd = -1;

bool SetStorage(std::string filename) {
  default_fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0777);
  return default_fd != -1;
}

bool Cleanup() { return close(default_fd) == 0; }
}
