#ifndef _MMAP_ALLOCATOR_H
#define _MMAP_ALLOCATOR_H

#include <memory>
#include <string>
#include <set>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <map>
#include <unordered_map>

namespace mm {

const int kPageSize = sysconf(_SC_PAGE_SIZE);

template <class T>
class Allocator;

extern int default_fd;

template <typename T>
class Allocator {
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
  friend class Allocator;

  template <typename U>
  bool operator==(const Allocator<U>& other) const {
    return sizes_ == other.sizes_;
  }

  template <typename U>
  bool operator!=(const Allocator<U>& other) const {
    return !(*this == other);
  }

  template <class U>
  struct rebind {
    typedef Allocator<U> other;
  };

  Allocator() : sizes_(new SizeMap), free_blocks_(new FreeMap) {}

  template <class U>
  Allocator(const Allocator<U>& other)
      : sizes_(other.sizes_), free_blocks_(other.free_blocks_) {}

  T* allocate(size_t n) {
    // round up to multiple of page size
    size_t to_alloc = n * sizeof(T);
    if (to_alloc % kPageSize != 0) {
      to_alloc = ((to_alloc / kPageSize) + 1) * kPageSize;
    }

    auto& vec = (*free_blocks_)[to_alloc];
    if (!vec.empty()) {
      T* addr = static_cast<T*>(vec.back());
      vec.pop_back();
      return addr;
    }

    off_t previous_end = lseek(default_fd, 0, SEEK_END);

    if (lseek(default_fd, to_alloc - 1, SEEK_END) == -1) {
      return NULL;
    }

    if (write(default_fd, "", 1) != 1) {
      return NULL;
    }

    T* addr = static_cast<T*>(mmap(0, to_alloc, PROT_READ | PROT_WRITE,
                                   MAP_SHARED, default_fd, previous_end));
    (*sizes_)[addr] = to_alloc;
    return addr;
  }

  // We don't actually need a passed-in size for deallocate, so we have this function for convenience.
  void deallocate(T* p) {
    deallocate(p, 0);
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
  // a map from allocated addresses to the "actual" sizes of their allocations
  // (as opposed to just what was requested, as requests are rounded up the
  // nearest page size multiple)
  std::shared_ptr<SizeMap> sizes_;
  std::shared_ptr<FreeMap> free_blocks_;
};

bool SetStorage(std::string filename);
bool Cleanup();

// Convenience typedefs of STL types
template <typename T>
using vector = std::vector<T, Allocator<T>>;

template <class Key, class T, class Compare = std::less<Key>>
using map = std::map<Key, T, Compare, Allocator<std::pair<const Key, T>>>;

template <class T, class Compare = std::less<T>>
using set = std::set<T, Compare, Allocator<T>>;

template <class Key, class T, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
using unordered_map = std::unordered_map<Key, T, Hash, KeyEqual,
                                         Allocator<std::pair<const Key, T>>>;

template<class CharT, class Traits = std::char_traits<CharT>>
using basic_string = std::basic_string<CharT, Traits, Allocator<CharT>>;

using string = basic_string<char>;
using wstring = basic_string<wchar_t>;
using u16string = basic_string<char16_t>;
using u32string = basic_string<char32_t>;

};  // namespace mm

template <class T>
void* operator new(size_t size, mm::Allocator<T>& alloc) {
  return alloc.allocate(size);
}

template <class T>
void* operator new [](size_t size, mm::Allocator<T>& alloc) {
  return alloc.allocate(size);
} template <class T>
void operator delete(void* p, mm::Allocator<T>& alloc) {
  alloc.deallocate(static_cast<T*>(p));
}

#endif
