#include <unity.h>

#include "interval_cycle.h"

void setUp() {}
void tearDown() {}

static void test_minutes_table_is_1_5_8() {
  TEST_ASSERT_EQUAL_UINT8(3, interval::kCount);
  TEST_ASSERT_EQUAL_UINT32(1, interval::minutesAt(0));
  TEST_ASSERT_EQUAL_UINT32(5, interval::minutesAt(1));
  TEST_ASSERT_EQUAL_UINT32(8, interval::minutesAt(2));
}

static void test_duration_ms() {
  TEST_ASSERT_EQUAL_UINT32(60000UL, interval::durationMs(0));
  TEST_ASSERT_EQUAL_UINT32(300000UL, interval::durationMs(1));
  TEST_ASSERT_EQUAL_UINT32(480000UL, interval::durationMs(2));
}

static void test_next_cycles_and_wraps() {
  TEST_ASSERT_EQUAL_UINT8(1, interval::next(0));
  TEST_ASSERT_EQUAL_UINT8(2, interval::next(1));
  TEST_ASSERT_EQUAL_UINT8(0, interval::next(2));
}

static void test_sanitize_valid_values_pass_through() {
  TEST_ASSERT_EQUAL_UINT8(0, interval::sanitize(0));
  TEST_ASSERT_EQUAL_UINT8(1, interval::sanitize(1));
  TEST_ASSERT_EQUAL_UINT8(2, interval::sanitize(2));
}

static void test_sanitize_invalid_values_fall_back_to_1min() {
  TEST_ASSERT_EQUAL_UINT8(0, interval::sanitize(3));
  TEST_ASSERT_EQUAL_UINT8(0, interval::sanitize(255));
  TEST_ASSERT_EQUAL_UINT8(0, interval::sanitize(-1));
}

static void test_out_of_range_index_is_treated_as_default() {
  TEST_ASSERT_EQUAL_UINT32(1, interval::minutesAt(7));
  TEST_ASSERT_EQUAL_UINT8(1, interval::next(7));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_minutes_table_is_1_5_8);
  RUN_TEST(test_duration_ms);
  RUN_TEST(test_next_cycles_and_wraps);
  RUN_TEST(test_sanitize_valid_values_pass_through);
  RUN_TEST(test_sanitize_invalid_values_fall_back_to_1min);
  RUN_TEST(test_out_of_range_index_is_treated_as_default);
  return UNITY_END();
}
