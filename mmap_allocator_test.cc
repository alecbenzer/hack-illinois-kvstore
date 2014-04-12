#include <map>
#include <memory>
#include <gtest/gtest.h>
#include "mmap_allocator.h"

class MMapAllocatorTest : public testing::Test {
 protected:
  template <class T>
  std::unique_ptr<MMapAllocator<T>> make_allocator() {
    std::unique_ptr<MMapAllocator<T>> ptr(MMapAllocator<T>::New("test.db"));
    EXPECT_FALSE(ptr.get() == NULL);
    return ptr;
  }

  template <class T>
  std::vector<T, MMapAllocator<T>> make_vector(MMapAllocator<T>* alloc) {
    return std::vector<T, MMapAllocator<T>>(*alloc);
  }
};

TEST_F(MMapAllocatorTest, VectorInt) {
  auto alloc = make_allocator<int>();
  auto vec = make_vector<int>(alloc.get());

  for (int i = 0; i < 10; ++i) {
    vec.push_back(i);
  }

  EXPECT_EQ(10, vec.size());

  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(i, vec[i]);
  }
}

TEST_F(MMapAllocatorTest, VectorIntInitialized) {
  auto alloc = make_allocator<int>();
  std::vector<int, MMapAllocator<int>> vec({4, 42, 24, 7}, *alloc);

  EXPECT_EQ(4, vec.size());
  EXPECT_EQ(4, vec[0]);
  EXPECT_EQ(42, vec[1]);
  EXPECT_EQ(24, vec[2]);
  EXPECT_EQ(7, vec[3]);
}

TEST_F(MMapAllocatorTest, VectorDouble) {
  auto alloc = make_allocator<double>();
  auto vec = make_vector<double>(alloc.get());

  for (int i = 0; i < 10; ++i) {
    vec.push_back(sqrt(i));
  }

  EXPECT_EQ(10, vec.size());

  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(sqrt(i), vec[i]);
  }
}

TEST_F(MMapAllocatorTest, VectorString) {
  auto alloc = make_allocator<std::string>();
  auto vec = make_vector<std::string>(alloc.get());

  for (int i = 0; i < 10; ++i) {
    vec.push_back("Hello, world!");
  }

  EXPECT_EQ(10, vec.size());

  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ("Hello, world!", vec[i]);
  }
}

TEST_F(MMapAllocatorTest, MapStringString) {
  using std::pair;
  using std::string;
  using std::less;
  using std::map;

  auto alloc = make_allocator<pair<const string, string>>();
  map<string, string, less<string>, MMapAllocator<pair<const string, string>>>
      m(less<string>(), *alloc);

  m["Hello"] = "World";
  m["Goodbye"] = "All";
  m["2+2"] = "4";

  EXPECT_EQ(3, m.size());
  EXPECT_EQ("World", m["Hello"]);
}
