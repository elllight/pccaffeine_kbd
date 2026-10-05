#include <unity.h>

#include "host_set.h"

void setUp() {}
void tearDown() {}

static void test_starts_empty() {
  HostSet hs;
  TEST_ASSERT_EQUAL_UINT8(0, hs.count());
  TEST_ASSERT_FALSE(hs.full());
  TEST_ASSERT_EQUAL_UINT8(3, HostSet::kCapacity);
}

static void test_add_and_remove() {
  HostSet hs;
  TEST_ASSERT_TRUE(hs.add(1));
  TEST_ASSERT_TRUE(hs.add(2));
  TEST_ASSERT_EQUAL_UINT8(2, hs.count());
  TEST_ASSERT_TRUE(hs.remove(1));
  TEST_ASSERT_EQUAL_UINT8(1, hs.count());
  TEST_ASSERT_TRUE(hs.contains(2));
  TEST_ASSERT_FALSE(hs.contains(1));
}

static void test_duplicate_add_is_ignored() {
  // Re-encryption of an existing link fires onAuthenticationComplete again.
  HostSet hs;
  TEST_ASSERT_TRUE(hs.add(5));
  TEST_ASSERT_FALSE(hs.add(5));
  TEST_ASSERT_EQUAL_UINT8(1, hs.count());
}

static void test_capacity_is_enforced() {
  HostSet hs;
  hs.add(1);
  hs.add(2);
  TEST_ASSERT_TRUE(hs.add(3));
  TEST_ASSERT_TRUE(hs.full());
  TEST_ASSERT_FALSE(hs.add(4));
  TEST_ASSERT_EQUAL_UINT8(3, hs.count());
}

static void test_remove_unknown_handle_is_noop() {
  // A link that dropped before pairing completed was never added.
  HostSet hs;
  hs.add(1);
  TEST_ASSERT_FALSE(hs.remove(9));
  TEST_ASSERT_EQUAL_UINT8(1, hs.count());
}

static void test_slot_is_reusable_after_remove() {
  HostSet hs;
  hs.add(1);
  hs.add(2);
  hs.add(3);
  hs.remove(2);
  TEST_ASSERT_FALSE(hs.full());
  TEST_ASSERT_TRUE(hs.add(4));
  TEST_ASSERT_TRUE(hs.contains(4));
  TEST_ASSERT_EQUAL_UINT8(3, hs.count());
}

static void test_invalid_handle_is_rejected() {
  HostSet hs;
  TEST_ASSERT_FALSE(hs.add(HostSet::kNone));
  TEST_ASSERT_EQUAL_UINT8(0, hs.count());
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_starts_empty);
  RUN_TEST(test_add_and_remove);
  RUN_TEST(test_duplicate_add_is_ignored);
  RUN_TEST(test_capacity_is_enforced);
  RUN_TEST(test_remove_unknown_handle_is_noop);
  RUN_TEST(test_slot_is_reusable_after_remove);
  RUN_TEST(test_invalid_handle_is_rejected);
  return UNITY_END();
}
