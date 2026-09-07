#include <string.h>

#include <unity.h>

#include "clock_formatter.h"
#include "test_helpers.h"

void testClockFormatterFormatsTimeValues() {
  char hhmm[6] = {};
  char hhmmss[9] = {};
  TEST_ASSERT_EQUAL_UINT32(5, ClockFormatter::formatHHMM(makeTime(4, 5), hhmm, sizeof(hhmm)));
  TEST_ASSERT_EQUAL_STRING("04:05", hhmm);
  TEST_ASSERT_EQUAL_UINT32(8, ClockFormatter::formatHHMMSS(makeTime(4, 5, 6), hhmmss, sizeof(hhmmss)));
  TEST_ASSERT_EQUAL_STRING("04:05:06", hhmmss);
}

void testClockFormatterFormatsShortDate() {
  char date[16] = {};
  TEST_ASSERT_EQUAL_UINT32(10, ClockFormatter::formatShortDate(makeTime(15, 1, 0, 0, 6, 9), date, sizeof(date)));
  TEST_ASSERT_EQUAL_STRING("Sun, Sep 6", date);
}

void testClockFormatterHandlesNullAndZeroBuffers() {
  char buffer[6] = "keep";
  TEST_ASSERT_EQUAL_UINT32(0, ClockFormatter::formatHHMM(makeTime(1, 2), nullptr, sizeof(buffer)));
  TEST_ASSERT_EQUAL_UINT32(0, ClockFormatter::formatHHMM(makeTime(1, 2), buffer, 0));
  TEST_ASSERT_EQUAL_STRING("keep", buffer);
}

void testClockFormatterTruncatesSafely() {
  char buffer[5] = {};
  TEST_ASSERT_EQUAL_UINT32(5, ClockFormatter::formatHHMM(makeTime(12, 34), buffer, sizeof(buffer)));
  TEST_ASSERT_EQUAL_STRING("12:3", buffer);
}
