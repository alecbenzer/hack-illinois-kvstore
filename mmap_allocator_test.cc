#include <map>
#include <unordered_map>
#include <memory>
#include <gtest/gtest.h>
#include "mmap_allocator.h"

using std::equal_to;
using std::hash;
using std::less;
using std::map;
using std::pair;
using std::string;
using std::to_string;
using std::unordered_map;
using std::unique_ptr;

class MMapAllocatorTest : public testing::Test {
 protected:
  template <class T>
  unique_ptr<MMapAllocator<T>> make_allocator() {
    return unique_ptr<MMapAllocator<T>>(MMapAllocator<T>::New("test.db"));
  }

  template <class T>
  std::vector<T, MMapAllocator<T>> make_vector(MMapAllocator<T>* alloc) {
    return std::vector<T, MMapAllocator<T>>(*alloc);
  }
};

TEST_F(MMapAllocatorTest, VectorInt) {
  auto alloc = make_allocator<int>();
  ASSERT_FALSE(alloc.get() == NULL);
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
  ASSERT_FALSE(alloc.get() == NULL);
  std::vector<int, MMapAllocator<int>> vec({4, 42, 24, 7}, *alloc);

  EXPECT_EQ(4, vec.size());
  EXPECT_EQ(4, vec[0]);
  EXPECT_EQ(42, vec[1]);
  EXPECT_EQ(24, vec[2]);
  EXPECT_EQ(7, vec[3]);
}

TEST_F(MMapAllocatorTest, VectorDouble) {
  auto alloc = make_allocator<double>();
  ASSERT_FALSE(alloc.get() == NULL);
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
  auto alloc = make_allocator<string>();
  ASSERT_FALSE(alloc.get() == NULL);
  auto vec = make_vector<string>(alloc.get());

  for (int i = 0; i < 10; ++i) {
    vec.push_back("Hello, world!");
  }

  EXPECT_EQ(10, vec.size());

  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ("Hello, world!", vec[i]);
  }
}

TEST_F(MMapAllocatorTest, MapStringString) {
  auto alloc = make_allocator<pair<const string, string>>();
  ASSERT_FALSE(alloc.get() == NULL);
  map<string, string, less<string>, MMapAllocator<pair<const string, string>>>
      m(less<string>(), *alloc);

  m["Hello"] = "World";
  m["Goodbye"] = "All";
  m["2+2"] = "4";

  EXPECT_EQ(3, m.size());
  EXPECT_EQ("World", m["Hello"]);
}

TEST_F(MMapAllocatorTest, HashStringString) {
  auto alloc = make_allocator<pair<const string, string>>();
  ASSERT_FALSE(alloc.get() == NULL);
  unordered_map<string, string, hash<string>, equal_to<string>,
                MMapAllocator<pair<const string, string>>> m(10, hash<string>(),
                                                             equal_to<string>(),
                                                             *alloc);

  m["Hello"] = "World";
  m["Goodbye"] = "All";
  m["2+2"] = "4";

  EXPECT_EQ(3, m.size());
  EXPECT_EQ("World", m["Hello"]);
}

TEST_F(MMapAllocatorTest, Nested) {
  using Vector = std::vector<string, MMapAllocator<string>>;
  using Hash =
      std::unordered_map<string, Vector, hash<string>, equal_to<string>,
                         MMapAllocator<pair<const string, Vector>>>;

  unique_ptr<Hash::allocator_type> hash_alloc(
      Hash::allocator_type::New("test.db"));
  Vector::allocator_type vector_alloc(*hash_alloc);

  auto m = Hash(10, hash<string>(), equal_to<string>(), *hash_alloc);
  EXPECT_EQ(0, m.size());

  m.insert(make_pair("foo", Vector(vector_alloc)));
  EXPECT_EQ(1, m.size());
  m.insert(make_pair("bar", Vector(vector_alloc)));
  EXPECT_EQ(2, m.size());

  EXPECT_EQ(0, m.at("foo").size());
  for (int i = 0; i < 10; ++i) {
    m.at("foo").push_back(to_string(i));
  }
  EXPECT_EQ(10, m.at("foo").size());

  EXPECT_EQ(0, m.at("bar").size());
  for (int i = 0; i < 10; ++i) {
    m.at("bar").push_back(to_string(i * i));
  }
  EXPECT_EQ(10, m.at("bar").size());

  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(to_string(i), m.at("foo")[i]);
  }
  for (int i = 0; i < 10; ++i) {
    EXPECT_EQ(to_string(i * i), m.at("bar")[i]);
  }
}
