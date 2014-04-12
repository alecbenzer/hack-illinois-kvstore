#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>

template <class T>
class MMapAllocator : public std::allocator<T> {
 public:
  typedef T *pointer;
  typedef const T *const_pointer;
  // typedef T &reference;
  // typedef const T &const_reference;
  // typedef T value_type;
  typedef size_t size_type;
  // typedef ptrdiff_t difference_type;
  typedef off_t offset_type;

  static const size_t BASE = 8;

  static MMapAllocator *New(std::string filename) {
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
      return NULL;
    }

    return new MMapAllocator(fd, sysconf(_SC_PAGE_SIZE));
  }

  pointer allocate(size_type n, const void *hint = 0) {
    size_t bytes_needed = sizeof(T) * n;
    size_t actual_bytes;
    if (bytes_needed % page_size_ != 0) {
      actual_bytes = ((bytes_needed / page_size_) + 1) * page_size_;
    } else {
      actual_bytes = bytes_needed;
    }

    off_t previous_end = lseek(fd_, 0, SEEK_END);
    if (lseek(fd_, actual_bytes - 1, SEEK_END) == -1) {
      return NULL;
    }

    if (write(fd_, "", 1) != 1) {
      return NULL;
    }

    return static_cast<T *>(mmap(0, actual_bytes, PROT_READ | PROT_WRITE,
                                 MAP_SHARED, fd_, previous_end));
  }

  void deallocate(pointer p, size_type n) {}

 private:
  MMapAllocator(int fd, long page_size) : fd_(fd), page_size_(page_size) {}

  int fd_;
  long page_size_;
};
