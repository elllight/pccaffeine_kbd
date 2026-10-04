#include <string.h>
#include <unity.h>

#include "view_math.h"

void setUp() {}
void tearDown() {}

static void test_consumed_degrees() {
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, view::consumedDegrees(1.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 180.0f, view::consumedDegrees(0.5f));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 360.0f, view::consumedDegrees(0.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, view::consumedDegrees(1.7f));     // clamp
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 360.0f, view::consumedDegrees(-0.2f));  // clamp
}

static void test_clockwise_angle_from_12_oclock() {
  // dx right, dy down (screen coordinates)
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 0.0f, view::clockAngleDeg(0, -10));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 90.0f, view::clockAngleDeg(10, 0));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 180.0f, view::clockAngleDeg(0, 10));
  TEST_ASSERT_FLOAT_WITHIN(0.5f, 270.0f, view::clockAngleDeg(-10, 0));
}

static void test_full_pie_fills_everything_inside_radius() {
  TEST_ASSERT_TRUE(view::pieFilled(0, -10, 18.0f, 1.0f));
  TEST_ASSERT_TRUE(view::pieFilled(10, 0, 18.0f, 1.0f));
  TEST_ASSERT_TRUE(view::pieFilled(-10, 0, 18.0f, 1.0f));
  TEST_ASSERT_FALSE(view::pieFilled(18, 18, 18.0f, 1.0f));  // outside circle
}

static void test_half_pie_keeps_left_half() {
  // Elapsed time eats the pie clockwise from 12 o'clock: at 50% the right half is gone.
  TEST_ASSERT_FALSE(view::pieFilled(10, 0, 18.0f, 0.5f));   // 3 o'clock consumed
  TEST_ASSERT_TRUE(view::pieFilled(-10, 0, 18.0f, 0.5f));   // 9 o'clock remains
  TEST_ASSERT_TRUE(view::pieFilled(-1, -10, 18.0f, 0.5f));  // just left of 12 remains
}

static void test_empty_pie_has_no_pixels() {
  TEST_ASSERT_FALSE(view::pieFilled(-10, 0, 18.0f, 0.0f));
  TEST_ASSERT_FALSE(view::pieFilled(0, 10, 18.0f, 0.0f));
}

static void test_format_mss_rounds_up_seconds() {
  char buf[8];
  view::formatMSS(480000UL, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("8:00", buf);
  view::formatMSS(59000UL, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("0:59", buf);
  view::formatMSS(58001UL, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("0:59", buf);
  view::formatMSS(0UL, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("0:00", buf);
  view::formatMSS(299999UL, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("5:00", buf);
}

static void test_interval_label() {
  char buf[8];
  view::formatIntervalLabel(1, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[1m]", buf);
  view::formatIntervalLabel(8, buf, sizeof(buf));
  TEST_ASSERT_EQUAL_STRING("[8m]", buf);
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_consumed_degrees);
  RUN_TEST(test_clockwise_angle_from_12_oclock);
  RUN_TEST(test_full_pie_fills_everything_inside_radius);
  RUN_TEST(test_half_pie_keeps_left_half);
  RUN_TEST(test_empty_pie_has_no_pixels);
  RUN_TEST(test_format_mss_rounds_up_seconds);
  RUN_TEST(test_interval_label);
  return UNITY_END();
}
