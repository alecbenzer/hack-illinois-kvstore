#include <memory>
#include <gtest/gtest.h>
#include "memory_map.h"

TEST(MemoryMapTest, Size) {
  std::unique_ptr<MemoryMap<int>> map(MemoryMap<int>::Create("test.db", 10));
  ASSERT_FALSE(map.get() == NULL);

  EXPECT_EQ(map->size(), 10);

  map.reset(MemoryMap<int>::Open("test.db"));
  ASSERT_FALSE(map.get() == NULL);

  EXPECT_EQ(map->size(), 10);
}

TEST(MemoryMapTest, GetSet) {
  std::unique_ptr<MemoryMap<int>> map(MemoryMap<int>::Create("test.db", 10));
  ASSERT_FALSE(map.get() == NULL);

  for (int i = 0; i < map->size(); ++i) {
    map->Set(i, i * i);
  }

  for (int i = 0; i < map->size(); ++i) {
    EXPECT_EQ(i * i, *map->Get(i));
  }
}

TEST(MemoryMapTest, GetSetReOpen) {
  std::unique_ptr<MemoryMap<int>> map(MemoryMap<int>::Create("test.db", 10));
  ASSERT_FALSE(map.get() == NULL);

  for (int i = 0; i < map->size(); ++i) {
    map->Set(i, i * i);
  }

  map.reset(MemoryMap<int>::Open("test.db"));
  ASSERT_FALSE(map.get() == NULL);

  for (int i = 0; i < map->size(); ++i) {
    EXPECT_EQ(i * i, *map->Get(i));
  }
}
