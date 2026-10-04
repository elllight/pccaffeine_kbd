#include <unity.h>

#include "debouncer.h"

void setUp() {}
void tearDown() {}

static void test_clean_press_emits_one_event_after_stable_time() {
  Debouncer db(30);
  TEST_ASSERT_FALSE(db.update(false, 0));
  TEST_ASSERT_FALSE(db.update(true, 100));
  TEST_ASSERT_FALSE(db.update(true, 129));
  TEST_ASSERT_TRUE(db.update(true, 130));
  TEST_ASSERT_TRUE(db.pressed());
}

static void test_bounce_is_rejected() {
  Debouncer db(30);
  db.update(false, 0);
  TEST_ASSERT_FALSE(db.update(true, 100));
  TEST_ASSERT_FALSE(db.update(false, 110));
  TEST_ASSERT_FALSE(db.update(true, 120));
  TEST_ASSERT_FALSE(db.update(false, 125));
  TEST_ASSERT_FALSE(db.update(false, 200));
  TEST_ASSERT_FALSE(db.pressed());
}

static void test_long_hold_emits_single_event() {
  Debouncer db(30);
  db.update(true, 0);
  int events = 0;
  for (uint32_t t = 0; t <= 5000; t += 5) {
    if (db.update(true, t)) events++;
  }
  TEST_ASSERT_EQUAL_INT(1, events);
}

static void test_release_does_not_emit_and_next_press_emits() {
  Debouncer db(30);
  db.update(true, 0);
  TEST_ASSERT_TRUE(db.update(true, 30));
  TEST_ASSERT_FALSE(db.update(false, 100));
  TEST_ASSERT_FALSE(db.update(false, 130));
  TEST_ASSERT_FALSE(db.pressed());
  TEST_ASSERT_FALSE(db.update(true, 200));
  TEST_ASSERT_TRUE(db.update(true, 230));
}

int main() {
  UNITY_BEGIN();
  RUN_TEST(test_clean_press_emits_one_event_after_stable_time);
  RUN_TEST(test_bounce_is_rejected);
  RUN_TEST(test_long_hold_emits_single_event);
  RUN_TEST(test_release_does_not_emit_and_next_press_emits);
  return UNITY_END();
}
