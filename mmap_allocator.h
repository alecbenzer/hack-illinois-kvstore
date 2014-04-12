#include <memory>
#include <string>
#include <vector>
#include <cmath>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

  template <class U>
  struct rebind {
    typedef MMapAllocator<U> other;
  };

  static MMapAllocator* New(std::string dirname) {
    struct stat st;
    if (stat(dirname.c_str(), &st) == 0) {
      // file already exists, make sure it's a directory
      if (!S_ISDIR(st.st_mode)) {
        return NULL;
      }
    } else {
      // doesn't exist
      return NULL;
    }

    return new MMapAllocator(dirname);
  }

  T* allocate(size_t n) {
    std::string filename = dirname_ + "/" + std::to_string(last_used_);
    last_used_ += 1;

    int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t)0777);
    if (fd == -1) {
      return NULL;
    }

    size_t bytes = n * sizeof(T);

    if (lseek(fd, bytes - 1, SEEK_END) == -1) {
      return NULL;
    }

    if (write(fd, "", 1) != 1) {
      return NULL;
    }

    return static_cast<T*>(
        mmap(0, bytes, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
  }

  void deallocate(T* p, size_t n) {}

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

  template <class U>
  MMapAllocator(const MMapAllocator<U>& other)
      : dirname_(other.dirname()), last_used_(other.last_used()) {}

  std::string dirname() const { return dirname_; }
  int last_used() const { return last_used_; }

 private:
  MMapAllocator(std::string dirname) : dirname_(dirname), last_used_(0) {}

  std::string dirname_;
  int last_used_;
};

// apparently we need this shit because templates are the devil
// see http://stackoverflow.com/a/21083096/598940
template <typename T, typename U>
inline bool operator==(const MMapAllocator<T>&, const MMapAllocator<U>&) {
  return true;
}

template <typename T, typename U>
inline bool operator!=(const MMapAllocator<T>& a, const MMapAllocator<U>& b) {
  return !(a == b);
}
