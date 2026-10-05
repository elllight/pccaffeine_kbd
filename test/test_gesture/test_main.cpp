#include <unity.h>

#include "button_gesture.h"

using Event = ButtonGesture::Event;

void setUp() {}
void tearDown() {}

// Presses at `start`, holds for `heldMs` (polling every 10 ms) and releases.
// Returns the events seen while held and on release.
static void pressFor(ButtonGesture& g, uint32_t start, uint32_t heldMs, int& shorts, int& longs) {
  shorts = longs = 0;
  for (uint32_t t = 0; t <= heldMs; t += 10) {
    const Event e = g.update(true, start + t);
    if (e == Event::Short) shorts++;
    if (e == Event::Long) longs++;
  }
  const Event e = g.update(false, start + heldMs);
  if (e == Event::Short) shorts++;
  if (e == Event::Long) longs++;
}

static void test_idle_produces_nothing() {
  ButtonGesture g;
  TEST_ASSERT_TRUE(g.update(false, 0) == Event::None);
  TEST_ASSERT_TRUE(g.update(false, 10000) == Event::None);
  TEST_ASSERT_FALSE(g.holding(10000));
}

static void test_short_press_fires_on_release_only() {
  ButtonGesture g;
  TEST_ASSERT_TRUE(g.update(true, 100) == Event::None);
  TEST_ASSERT_TRUE(g.update(true, 300) == Event::None);
  TEST_ASSERT_TRUE(g.update(false, 400) == Event::Short);
  TEST_ASSERT_TRUE(g.update(false, 500) == Event::None);
}

static void test_short_boundary_999_vs_1000() {
  ButtonGesture g;
  int shorts, longs;
  pressFor(g, 0, 990, shorts, longs);
  TEST_ASSERT_EQUAL_INT(1, shorts);
  g.update(true, 5000);
  TEST_ASSERT_TRUE(g.update(false, 5999) == Event::Short);  // held 999 ms
  g.update(true, 7000);
  TEST_ASSERT_TRUE(g.update(false, 8000) == Event::None);  // held 1000 ms -> cancel
}

static void test_hold_between_1_and_5_s_is_cancelled() {
  ButtonGesture g;
  int shorts, longs;
  pressFor(g, 0, 3000, shorts, longs);
  TEST_ASSERT_EQUAL_INT(0, shorts);
  TEST_ASSERT_EQUAL_INT(0, longs);
  g.update(true, 10000);
  TEST_ASSERT_TRUE(g.update(true, 14999) == Event::None);
  TEST_ASSERT_TRUE(g.update(false, 14999) == Event::None);  // 4999 ms
}

static void test_long_press_fires_once_while_held() {
  ButtonGesture g;
  TEST_ASSERT_TRUE(g.update(true, 0) == Event::None);
  TEST_ASSERT_TRUE(g.update(true, 4999) == Event::None);
  TEST_ASSERT_TRUE(g.update(true, 5000) == Event::Long);
  TEST_ASSERT_TRUE(g.update(true, 8000) == Event::None);
  TEST_ASSERT_TRUE(g.update(false, 9000) == Event::None);  // release after long: nothing
}

static void test_long_hold_counts_once() {
  ButtonGesture g;
  int shorts, longs;
  pressFor(g, 0, 20000, shorts, longs);
  TEST_ASSERT_EQUAL_INT(0, shorts);
  TEST_ASSERT_EQUAL_INT(1, longs);
}

static void test_countdown_display_values() {
  ButtonGesture g;
  g.update(true, 0);
  TEST_ASSERT_FALSE(g.holding(999));
  TEST_ASSERT_TRUE(g.holding(1000));
  TEST_ASSERT_EQUAL_UINT32(4, g.secondsToLong(1000));
  TEST_ASSERT_EQUAL_UINT32(1, g.secondsToLong(4001));
  TEST_ASSERT_EQUAL_UINT32(1, g.secondsToLong(4999));
  g.update(true, 5000);  // long fired
  TEST_ASSERT_FALSE(g.holding(5000));
  g.update(false, 6000);
  TEST_ASSERT_FALSE(g.holding(6000));
}

static void test_millis_wraparound() {
  ButtonGesture g;
  const uint32_t start = 0xFFFFFFFFUL - 2000UL;
  g.update(true, start);
  TEST_ASSERT_TRUE(g.update(true, start + 4999) == Event::None);
  TEST_ASSERT_TRUE(g.update(true, start + 5000) == Event::Long);  // wrapped
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_idle_produces_nothing);
  RUN_TEST(test_short_press_fires_on_release_only);
  RUN_TEST(test_short_boundary_999_vs_1000);
  RUN_TEST(test_hold_between_1_and_5_s_is_cancelled);
  RUN_TEST(test_long_press_fires_once_while_held);
  RUN_TEST(test_long_hold_counts_once);
  RUN_TEST(test_countdown_display_values);
  RUN_TEST(test_millis_wraparound);
  return UNITY_END();
}
