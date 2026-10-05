#include <unity.h>

#include "pending_links.h"

void setUp() {}
void tearDown() {}

static const uint32_t kTimeout = 30000;

static void test_no_links_nothing_expires() {
  PendingLinks p;
  uint16_t out[PendingLinks::kCapacity];
  TEST_ASSERT_EQUAL_UINT8(0, p.expired(100000, kTimeout, out));
}

static void test_link_expires_after_timeout_only() {
  PendingLinks p;
  uint16_t out[PendingLinks::kCapacity];
  TEST_ASSERT_TRUE(p.add(1, 1000));
  TEST_ASSERT_EQUAL_UINT8(0, p.expired(1000 + kTimeout - 1, kTimeout, out));
  TEST_ASSERT_EQUAL_UINT8(1, p.expired(1000 + kTimeout, kTimeout, out));
  TEST_ASSERT_EQUAL_UINT16(1, out[0]);
}

static void test_paired_link_is_removed_and_never_expires() {
  PendingLinks p;
  uint16_t out[PendingLinks::kCapacity];
  p.add(1, 0);
  TEST_ASSERT_TRUE(p.remove(1));  // pairing completed or link dropped
  TEST_ASSERT_EQUAL_UINT8(0, p.expired(kTimeout * 10, kTimeout, out));
}

static void test_only_old_links_expire() {
  PendingLinks p;
  uint16_t out[PendingLinks::kCapacity];
  p.add(1, 0);
  p.add(2, 20000);
  TEST_ASSERT_EQUAL_UINT8(1, p.expired(kTimeout + 5000, kTimeout, out));
  TEST_ASSERT_EQUAL_UINT16(1, out[0]);
}

static void test_capacity_and_duplicates() {
  PendingLinks p;
  for (uint16_t h = 0; h < PendingLinks::kCapacity; ++h) TEST_ASSERT_TRUE(p.add(h, 0));
  TEST_ASSERT_FALSE(p.add(99, 0));  // full
  p.remove(0);
  TEST_ASSERT_TRUE(p.add(7, 5));
  TEST_ASSERT_FALSE(p.add(7, 6));  // duplicate keeps the original timestamp
  uint16_t out[PendingLinks::kCapacity];
  TEST_ASSERT_EQUAL_UINT8(PendingLinks::kCapacity, p.expired(5 + kTimeout, kTimeout, out));
}

static void test_invalid_handle_rejected() {
  PendingLinks p;
  TEST_ASSERT_FALSE(p.add(PendingLinks::kNone, 0));
  TEST_ASSERT_FALSE(p.remove(PendingLinks::kNone));
}

static void test_millis_wraparound() {
  PendingLinks p;
  uint16_t out[PendingLinks::kCapacity];
  const uint32_t start = 0xFFFFFFFFUL - 1000UL;
  p.add(3, start);
  TEST_ASSERT_EQUAL_UINT8(0, p.expired(start + kTimeout - 1, kTimeout, out));
  TEST_ASSERT_EQUAL_UINT8(1, p.expired(start + kTimeout, kTimeout, out));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_no_links_nothing_expires);
  RUN_TEST(test_link_expires_after_timeout_only);
  RUN_TEST(test_paired_link_is_removed_and_never_expires);
  RUN_TEST(test_only_old_links_expire);
  RUN_TEST(test_capacity_and_duplicates);
  RUN_TEST(test_invalid_handle_rejected);
  RUN_TEST(test_millis_wraparound);
  return UNITY_END();
}
