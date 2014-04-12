#include <memory>
#include <gtest/gtest.h>
#include "mmap_allocator.h"

class MMapAllocatorTest : public testing::Test {
 protected:
  virtual void SetUp() {
    mmap_alloc_.reset(MMapAllocator<int>::New("test.db"));
  }
 
 std::unique_ptr<MMapAllocator<int>> mmap_alloc_;
};

TEST_F(MMapAllocatorTest, Vector) {
  std::vector<int, MMapAllocator<int>> vec(*mmap_alloc_);

  for (int i = 0; i < 10; ++i) {
    vec.push_back(i);
  }

  EXPECT_EQ(10, vec.size());

  for (int i = 0; i < vec.size(); ++i) {
    EXPECT_EQ(i, vec[i]);
  }
}

TEST_F(MMapAllocatorTest, VectorInitialized) {
  std::vector<int, MMapAllocator<int>> vec({4, 42, 24, 7}, *mmap_alloc_);

  EXPECT_EQ(4, vec.size());
  EXPECT_EQ(4, vec[0]);
  EXPECT_EQ(42, vec[1]);
  EXPECT_EQ(24, vec[2]);
  EXPECT_EQ(7, vec[3]);
}
