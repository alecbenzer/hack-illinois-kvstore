#ifndef MEMORY_MAP_H
#define MEMORY_MAP_H

#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <unistd.h>

template <class T>
class MemoryMap {
 public:
  static MemoryMap* Create(std::string filename, size_t size) {
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);
    if (fd == -1) {
      return NULL;
    }

    size_t real_size = sizeof(T) * size;

    if (lseek(fd, real_size - 1, SEEK_SET) == -1) {
      close(fd);
      return NULL;
    }

    if (write(fd, "", 1) != 1) {
      close(fd);
      return NULL;
    }

    T* array = static_cast<T*>(
        mmap(0, real_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (array == MAP_FAILED) {
      close(fd);
      return NULL;
    }

    return new MemoryMap(array, size);
  }

  static MemoryMap* Open(std::string filename) {
    int fd = open(filename.c_str(), O_RDWR, (mode_t)0600);
    if (fd == -1) {
      return NULL;
    }

    size_t real_size = lseek(fd, 0, SEEK_END);

    if (real_size % sizeof(T) != 0) {
      return NULL;
    }
    size_t size = real_size / sizeof(T);

    T* array = static_cast<T*>(
        mmap(0, real_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
    if (array == MAP_FAILED) {
      close(fd);
      return NULL;
    }

    return new MemoryMap(array, size);
  }

  void Set(int index, const T& key) { array_[index] = key; }

  T* Get(int index) { return &array_[index]; }

  bool Close() { return munmap(array_, size_) != -1; }

  size_t size() const { return size_; }

 private:
  MemoryMap(T* array, size_t size) : array_(array), size_(size) {}

  T* array_;
  size_t size_;
};

#endif  // MEMORY_MAP_H
