#include <unity.h>

#include "alarm_service.h"
#include "test_helpers.h"

void testAlarmServiceDefaults() {
  AlarmService alarm_service;
  TEST_ASSERT_FALSE(alarm_service.isEnabled(0));
  TEST_ASSERT_FALSE(alarm_service.isRinging(0));
  TEST_ASSERT_EQUAL_UINT8(7, alarm_service.hour(0));
  TEST_ASSERT_EQUAL_UINT8(0, alarm_service.minute(0));
  TEST_ASSERT_EQUAL_UINT8(AlarmService::kEveryDayMask, alarm_service.weekdayMask(0));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(kDefaultAlarmRingtone), static_cast<uint8_t>(alarm_service.ringtone(0)));
}

void testAlarmServiceAddsAlarmWithRequestedTime() {
  AlarmService alarm_service;
  TEST_ASSERT_TRUE(alarm_service.addAlarm(23, 59));
  TEST_ASSERT_EQUAL_UINT8(2, alarm_service.count());
  TEST_ASSERT_EQUAL_UINT8(23, alarm_service.hour(1));
  TEST_ASSERT_EQUAL_UINT8(59, alarm_service.minute(1));
  TEST_ASSERT_FALSE(alarm_service.isEnabled(1));
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(kDefaultAlarmRingtone), static_cast<uint8_t>(alarm_service.ringtone(1)));
}

void testAlarmServiceKeepsOnlyTheLatestSimultaneousAlarmRinging() {
  AlarmService alarm_service;
  TEST_ASSERT_TRUE(alarm_service.addAlarm(7, 0));
  alarm_service.setEnabled(0, true);
  alarm_service.setEnabled(1, true);
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(7, 0)));
  TEST_ASSERT_FALSE(alarm_service.isRinging(0));
  TEST_ASSERT_TRUE(alarm_service.isRinging(1));
  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.dismissAllRinging());
  TEST_ASSERT_FALSE(alarm_service.hasRingingAlarm());
  TEST_ASSERT_TRUE(alarm_service.isEnabled(0));
  TEST_ASSERT_TRUE(alarm_service.isEnabled(1));
}

void testAlarmServiceStoresPerAlarmRingtone() {
  AlarmService alarm_service;
  alarm_service.setRingtone(0, AlarmRingtone::Urgent);
  TEST_ASSERT_EQUAL_UINT8(static_cast<uint8_t>(AlarmRingtone::Urgent), static_cast<uint8_t>(alarm_service.ringtone(0)));
}

void testAlarmServiceWrapsHourAndMinute() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 25, 61);
  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.hour(0));
  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.minute(0));
}

void testAlarmServiceDoesNotRingWhenDisabled() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30)));
  TEST_ASSERT_FALSE(alarm_service.isRinging(0));
}

void testAlarmServiceTriggersOnlyOncePerMinute() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30)));
  TEST_ASSERT_TRUE(alarm_service.isRinging(0));
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 10)));
  TEST_ASSERT_TRUE(alarm_service.isRinging(0));
}

void testAlarmServiceDismissSuppressesSameMinuteButAllowsLaterRetrigger() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30)));
  alarm_service.dismiss(0);
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 20)));
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 31)));
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30)));
}

void testAlarmServiceOnlyRingsOnSelectedWeekdays() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 0, 0)));
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 31, 0, 0)));
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30, 0, 1)));
}

void testAlarmServiceDoesNotRingWhenNoWeekdaysAreSelected() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setWeekdayMask(0, 0);
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 0, 1)));
}

void testAlarmServiceFindsNextEnabledOccurrenceUsingRepeatRules() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 0, 15);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.addAlarm(19, 30));
  alarm_service.setWeekdayMask(1, static_cast<uint8_t>(1u << 0));
  alarm_service.setEnabled(1, true);
  AlarmService::NextOccurrence next = {};
  TEST_ASSERT_TRUE(alarm_service.nextOccurrence(makeTime(12, 0, 0, 0), next));
  TEST_ASSERT_EQUAL_UINT8(1, next.alarm_index);
  TEST_ASSERT_EQUAL_UINT8(0, next.days_from_now);
  TEST_ASSERT_EQUAL_UINT16(450, next.minutes_until);
  TEST_ASSERT_EQUAL_UINT8(19, next.hour);
  TEST_ASSERT_EQUAL_UINT8(30, next.minute);
}

void testAlarmServiceFindsTomorrowWhenTodaysAlarmHasPassed() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));
  alarm_service.setEnabled(0, true);
  AlarmService::NextOccurrence next = {};
  TEST_ASSERT_TRUE(alarm_service.nextOccurrence(makeTime(7, 0, 0, 0), next));
  TEST_ASSERT_EQUAL_UINT8(1, next.days_from_now);
  TEST_ASSERT_EQUAL_UINT16(1410, next.minutes_until);
}

void testAlarmServiceFindsNextWeeksOnlySelectedWeekdayAfterItHasPassed() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 0));
  alarm_service.setEnabled(0, true);
  AlarmService::NextOccurrence next = {};
  TEST_ASSERT_TRUE(alarm_service.nextOccurrence(makeTime(7, 0, 0, 0), next));
  TEST_ASSERT_EQUAL_UINT8(7, next.days_from_now);
  TEST_ASSERT_EQUAL_UINT16(10050, next.minutes_until);
}

void testAlarmServiceSnoozesRingingAlarmForTenMinutes() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30, 0, 0)));
  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.snoozeAllRinging(makeTime(6, 30, 0, 0)));
  TEST_ASSERT_FALSE(alarm_service.isRinging(0));
  TEST_ASSERT_TRUE(alarm_service.isEnabled(0));
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 39, 0, 0)));
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 40, 0, 0)));
  TEST_ASSERT_TRUE(alarm_service.isRinging(0));
}
