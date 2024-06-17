#include<gtest/gtest.h>
#include"openfhe.h"

using namespace lbcrypto;

void assert_load_table(AllocationTable& tbl, size_t begin, size_t vals) {
  size_t slot;
  for(ValueId i = 0; i < vals; i++) {
    ValueId val = begin + i;
    ASSERT_TRUE(tbl.alloc_val(val, slot)); 
  }

  for(ValueId i = 0; i < vals; i++) {
    ValueId val = begin + i;
    ASSERT_TRUE(tbl.get_loc(val, slot)); 
  }
}

TEST(allocation_table, create_and_load)  {
  AllocationTable tbl { 10 };

  size_t slot;
  assert_load_table(tbl, 100, 10);
  ASSERT_FALSE(tbl.alloc_val(10000, slot));               // there should be no room at this point
  ASSERT_EQ(tbl.total_slots(), 10);
  ASSERT_EQ(tbl.allocated_slots(), 10);
}

TEST(allocation_table, free) {
  AllocationTable tbl { 10 };
  assert_load_table(tbl, 100, 10);
  tbl.free_val(100);
  size_t slot, slot2;
  ASSERT_EQ(tbl.allocated_slots(), 9);
  ASSERT_TRUE(tbl.alloc_val(200, slot));
  ASSERT_TRUE(tbl.get_loc(200, slot2));
  ASSERT_EQ(slot, slot2);
}

TEST(allocation_table, clear) {
  AllocationTable tbl { 10 };
  assert_load_table(tbl, 100, 10);
  tbl.clear();
  ASSERT_EQ(tbl.allocated_slots(), 0);
  assert_load_table(tbl, 100, 10);
}

TEST(allocation_table, fresh) {
  AllocationTable tbl { 10 };
  assert_load_table(tbl, 100, 9);
  size_t slot1, slot2;
  tbl.free_val(100);
  ASSERT_TRUE(tbl.alloc_val_fresh(200, slot1));
  ASSERT_FALSE(tbl.alloc_val_fresh(201, slot2));
  ASSERT_TRUE(tbl.alloc_val(201, slot2));
}
