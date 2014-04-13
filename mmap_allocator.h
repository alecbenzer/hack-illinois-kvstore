#ifndef _MMAP_ALLOCATOR_H
#define _MMAP_ALLOCATOR_H

#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

const int page_size = sysconf(_SC_PAGE_SIZE);

template <typename T>
class MMapAllocator {
 public:
  using value_type = T;
  using pointer = T*;
  using const_pointer = const T*;
  using reference = T&;
  using const_reference = const T&;
  using size_type = std::size_t;
  using difference_type = off_t;

  using SizeMap = std::map<void*, size_type>;
  using FreeMap = std::map<size_type, std::vector<void*>>;

  template <class U>
  friend class MMapAllocator;

  template <class U>
  struct rebind {
    typedef MMapAllocator<U> other;
  };

  static MMapAllocator* New(std::string filename) {
    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0777);
    if (fd == -1) {
      return NULL;
    }

    return new MMapAllocator(fd, new SizeMap(), new FreeMap());
  }

  template <class U>
  MMapAllocator(const MMapAllocator<U>& other)
      : fd_(other.fd_),
        sizes_(other.sizes_),
        free_blocks_(other.free_blocks_) {}

  T* allocate(size_t n) {
    // round up to multiple of page size
    size_t to_alloc = n * sizeof(T);
    if (to_alloc % page_size != 0) {
      to_alloc = ((to_alloc / page_size) + 1) * page_size;
    }

    auto& vec = (*free_blocks_)[to_alloc];
    if (!vec.empty()) {
      T* addr = static_cast<T*>(vec.back());
      vec.pop_back();
      return addr;
    }

    off_t previous_end = lseek(fd_, 0, SEEK_END);

    if (lseek(fd_, to_alloc - 1, SEEK_END) == -1) {
      return NULL;
    }

    if (write(fd_, "", 1) != 1) {
      return NULL;
    }
    T* addr = static_cast<T*>(mmap(0, to_alloc, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, fd_, previous_end));
    (*sizes_)[addr] = to_alloc;
    return addr;
  }

  void deallocate(T* p, size_t n) {
    auto to_free = (*sizes_)[(void*)p];
    (*free_blocks_)[to_free].push_back((void*)p);
  }

  void construct(pointer p, const_reference val) { new ((void*)p) T(val); }

  void destroy(pointer p) { p->~T(); }

  template <class U, class... Args>
  void construct(U* p, Args&&... args) {
    ::new ((void*)p) U(std::forward<Args>(args)...);
  }

  template <class U>
  void destroy(U* p) {
    p->~U();
  }

 private:
  MMapAllocator(int fd, SizeMap* sizes, FreeMap* free_blocks)
      : fd_(fd), sizes_(sizes), free_blocks_(free_blocks) {}

  int fd_;
  // a map from allocated addresses to the "actual" sizes of their allocations
  // (as opposed to just what was requested, as requests are rounded up the
  // nearest page size multiple)
  std::shared_ptr<SizeMap> sizes_;
  std::shared_ptr<FreeMap> free_blocks_;
};

template <typename T, typename U>
inline bool operator==(const MMapAllocator<T>& a, const MMapAllocator<U>& b) {
  return a.fd() == b.fd();
}

template <typename T, typename U>
inline bool operator!=(const MMapAllocator<T>& a, const MMapAllocator<U>& b) {
  return !(a == b);
}

#endif
