#include <unity.h>

#include "countdown.h"

void setUp() {}
void tearDown() {}

static const uint32_t kMin = 60000UL;

static void test_starts_paused_and_full() {
  Countdown cd(kMin);
  TEST_ASSERT_FALSE(cd.running());
  TEST_ASSERT_EQUAL_UINT32(kMin, cd.remainingMs(12345));
  TEST_ASSERT_FALSE(cd.update(999999));
}

static void test_runs_only_while_connected() {
  Countdown cd(kMin);
  cd.setConnected(true, 1000);
  TEST_ASSERT_TRUE(cd.running());
  TEST_ASSERT_EQUAL_UINT32(kMin - 10000, cd.remainingMs(11000));
  cd.setConnected(false, 11000);
  TEST_ASSERT_FALSE(cd.running());
  TEST_ASSERT_FALSE(cd.update(1000 + kMin * 5));
}

static void test_reconnect_restarts_from_full() {
  Countdown cd(kMin);
  cd.setConnected(true, 0);
  cd.update(30000);
  cd.setConnected(false, 30000);
  cd.setConnected(true, 50000);
  TEST_ASSERT_EQUAL_UINT32(kMin, cd.remainingMs(50000));
  TEST_ASSERT_FALSE(cd.update(50000 + kMin - 1));
  TEST_ASSERT_TRUE(cd.update(50000 + kMin));
}

static void test_repeated_connected_true_does_not_reset() {
  Countdown cd(kMin);
  cd.setConnected(true, 0);
  cd.setConnected(true, 20000);
  TEST_ASSERT_EQUAL_UINT32(kMin - 20000, cd.remainingMs(20000));
}

static void test_fires_exactly_once_then_restarts() {
  Countdown cd(kMin);
  cd.setConnected(true, 0);
  TEST_ASSERT_FALSE(cd.update(kMin - 1));
  TEST_ASSERT_TRUE(cd.update(kMin + 5));
  TEST_ASSERT_FALSE(cd.update(kMin + 6));
  TEST_ASSERT_EQUAL_UINT32(kMin, cd.remainingMs(kMin + 5));
  TEST_ASSERT_TRUE(cd.update(kMin + 5 + kMin));
}

static void test_interval_change_resets_to_new_full_duration() {
  Countdown cd(kMin);
  cd.setConnected(true, 0);
  cd.update(40000);
  cd.setDuration(5 * kMin, 40000);
  TEST_ASSERT_EQUAL_UINT32(5 * kMin, cd.durationMs());
  TEST_ASSERT_EQUAL_UINT32(5 * kMin, cd.remainingMs(40000));
  TEST_ASSERT_FALSE(cd.update(40000 + 5 * kMin - 1));
  TEST_ASSERT_TRUE(cd.update(40000 + 5 * kMin));
}

static void test_interval_change_while_paused_keeps_paused() {
  Countdown cd(kMin);
  cd.setDuration(8 * kMin, 100);
  TEST_ASSERT_FALSE(cd.running());
  TEST_ASSERT_EQUAL_UINT32(8 * kMin, cd.remainingMs(100));
}

static void test_millis_wraparound() {
  Countdown cd(kMin);
  const uint32_t start = 0xFFFFFFFFUL - 10000UL;
  cd.setConnected(true, start);
  TEST_ASSERT_EQUAL_UINT32(kMin - 20001, cd.remainingMs(start + 20001));  // wrapped
  TEST_ASSERT_FALSE(cd.update(start + kMin - 1));
  TEST_ASSERT_TRUE(cd.update(start + kMin));
}

static void test_remaining_fraction() {
  Countdown cd(kMin);
  TEST_ASSERT_EQUAL_FLOAT(1.0f, cd.remainingFraction(0));
  cd.setConnected(true, 0);
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.5f, cd.remainingFraction(30000));
  TEST_ASSERT_FLOAT_WITHIN(0.0001f, 0.0f, cd.remainingFraction(kMin + 100));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_starts_paused_and_full);
  RUN_TEST(test_runs_only_while_connected);
  RUN_TEST(test_reconnect_restarts_from_full);
  RUN_TEST(test_repeated_connected_true_does_not_reset);
  RUN_TEST(test_fires_exactly_once_then_restarts);
  RUN_TEST(test_interval_change_resets_to_new_full_duration);
  RUN_TEST(test_interval_change_while_paused_keeps_paused);
  RUN_TEST(test_millis_wraparound);
  RUN_TEST(test_remaining_fraction);
  return UNITY_END();
}
