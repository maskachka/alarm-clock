#include <string.h>

#include <unity.h>

#include "alarm_service.h"
#include "clock_app_controller.h"
#include "clock_formatter.h"

namespace {

ClockTime makeTime(uint8_t hour, uint8_t minute, uint8_t second = 0, uint8_t weekday = 0, uint8_t day = 0,
                   uint8_t month = 0) {
  ClockTime time_value = {hour, minute, second, weekday, day, month};
  return time_value;
}

void testAlarmServiceDefaults() {
  AlarmService alarm_service;

  TEST_ASSERT_FALSE(alarm_service.isEnabled());
  TEST_ASSERT_FALSE(alarm_service.isRinging());
  TEST_ASSERT_EQUAL_UINT8(7, alarm_service.hour());
  TEST_ASSERT_EQUAL_UINT8(0, alarm_service.minute());
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

  alarm_service.setAlarm(25, 61);

  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.hour());
  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.minute());
}

void testAlarmServiceDoesNotRingWhenDisabled() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);

  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30)));
  TEST_ASSERT_FALSE(alarm_service.isRinging());
}

void testAlarmServiceTriggersOnlyOncePerMinute() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(true);

  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30)));
  TEST_ASSERT_TRUE(alarm_service.isRinging());

  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 10)));
  TEST_ASSERT_TRUE(alarm_service.isRinging());
}

void testAlarmServiceDismissSuppressesSameMinuteButAllowsLaterRetrigger() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(true);

  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30)));
  alarm_service.dismiss();

  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 20)));
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 31)));
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30)));
}

void testAlarmServiceOnlyRingsOnSelectedWeekdays() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));  // Monday
  alarm_service.setEnabled(true);

  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 0, 0)));  // Sunday
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 31, 0, 0)));
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30, 0, 1)));  // Monday
}

void testAlarmServiceDoesNotRingWhenNoWeekdaysAreSelected() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setWeekdayMask(0, 0);
  alarm_service.setEnabled(true);

  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 30, 0, 1)));
}

void testAlarmServiceFindsNextEnabledOccurrenceUsingRepeatRules() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 15);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));  // Monday
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.addAlarm(19, 30));
  alarm_service.setWeekdayMask(1, static_cast<uint8_t>(1u << 0));  // Sunday
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
  alarm_service.setAlarm(6, 30);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));  // Monday
  alarm_service.setEnabled(0, true);

  AlarmService::NextOccurrence next = {};
  TEST_ASSERT_TRUE(alarm_service.nextOccurrence(makeTime(7, 0, 0, 0), next));  // Sunday
  TEST_ASSERT_EQUAL_UINT8(1, next.days_from_now);
  TEST_ASSERT_EQUAL_UINT16(1410, next.minutes_until);
}

void testAlarmServiceFindsNextWeeksOnlySelectedWeekdayAfterItHasPassed() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 0));  // Sunday
  alarm_service.setEnabled(0, true);

  AlarmService::NextOccurrence next = {};
  TEST_ASSERT_TRUE(alarm_service.nextOccurrence(makeTime(7, 0, 0, 0), next));
  TEST_ASSERT_EQUAL_UINT8(7, next.days_from_now);
  TEST_ASSERT_EQUAL_UINT16(10050, next.minutes_until);
}

void testAlarmServiceSnoozesRingingAlarmForTenMinutes() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 30, 0, 0)));

  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.snoozeAllRinging(makeTime(6, 30, 0, 0)));
  TEST_ASSERT_FALSE(alarm_service.isRinging());
  TEST_ASSERT_TRUE(alarm_service.isEnabled());
  TEST_ASSERT_FALSE(alarm_service.update(makeTime(6, 39, 0, 0)));
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(6, 40, 0, 0)));
  TEST_ASSERT_TRUE(alarm_service.isRinging());
}

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

void testControllerInitializeSetsDefaultViewState() {
  AlarmService alarm_service;
  ClockAppController controller(alarm_service);

  controller.initialize();

  TEST_ASSERT_EQUAL_STRING("--:--", controller.state().clock_text);
  TEST_ASSERT_EQUAL_STRING("07:00", controller.state().alarm_text);
  TEST_ASSERT_EQUAL_STRING("Set", controller.state().primary_button_text);
  TEST_ASSERT_FALSE(controller.state().alarm_enabled);
  TEST_ASSERT_FALSE(controller.state().alarm_settings_visible);
}

void testControllerRefreshUnavailableKeepsPlaceholderClock() {
  AlarmService alarm_service;
  ClockAppController controller(alarm_service);
  controller.initialize();

  const ClockAppEffects effects = controller.refresh(false, makeTime(0, 0));

  TEST_ASSERT_FALSE(effects.start_buzzer);
  TEST_ASSERT_FALSE(effects.stop_buzzer);
  TEST_ASSERT_FALSE(effects.update_buzzer);
  TEST_ASSERT_EQUAL_STRING("--:--", controller.state().clock_text);
}

void testControllerRefreshStartsAndUpdatesBuzzerWhenAlarmTriggers() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(true);
  ClockAppController controller(alarm_service);
  controller.initialize();

  const ClockAppEffects effects = controller.refresh(true, makeTime(6, 30, 0));

  TEST_ASSERT_TRUE(effects.start_buzzer);
  TEST_ASSERT_FALSE(effects.stop_buzzer);
  TEST_ASSERT_TRUE(effects.update_buzzer);
  TEST_ASSERT_EQUAL_STRING("06:30", controller.state().clock_text);
  TEST_ASSERT_EQUAL_STRING("06:30", controller.state().alarm_text);
  TEST_ASSERT_EQUAL_STRING("Dismiss", controller.state().primary_button_text);
  TEST_ASSERT_TRUE(controller.state().alarm_enabled);
}

void testControllerRefreshKeepsColonInClockText() {
  AlarmService alarm_service;
  ClockAppController controller(alarm_service);
  controller.initialize();

  controller.refresh(true, makeTime(6, 30, 1));

  TEST_ASSERT_EQUAL_STRING("06:30", controller.state().clock_text);
}

void testControllerLabelsTomorrowAlarmEvenWhenItIsLessThanTwentyFourHoursAway() {
  AlarmService alarm_service;
  alarm_service.setAlarm(5, 45);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));  // Monday
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();

  controller.refresh(true, makeTime(15, 17, 0, 0, 6, 9));  // Sunday, Sep 6

  TEST_ASSERT_EQUAL_STRING("Next alarm tomorrow in 14h 28m, at 05:45", controller.state().next_alarm_text);
}

void testControllerOmitsTodayAndZeroHoursFromNextAlarmText() {
  AlarmService alarm_service;
  alarm_service.setAlarm(21, 30);
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();

  controller.refresh(true, makeTime(20, 31, 0, 0, 6, 9));

  TEST_ASSERT_EQUAL_STRING("Next alarm in 59m, at 21:30", controller.state().next_alarm_text);
}

void testControllerPrioritizesSnoozeStatusInNextAlarmText() {
  AlarmService alarm_service;
  alarm_service.setAlarm(21, 36);
  alarm_service.setEnabled(0, true);
  TEST_ASSERT_TRUE(alarm_service.update(makeTime(21, 36, 0, 0)));
  TEST_ASSERT_EQUAL_UINT8(1, alarm_service.snoozeAllRinging(makeTime(21, 36, 0, 0)));
  ClockAppController controller(alarm_service);
  controller.initialize();

  controller.refresh(true, makeTime(21, 36, 0, 0));

  TEST_ASSERT_EQUAL_STRING("Snoozing for 10m...", controller.state().next_alarm_text);
}

void testControllerRefreshKeepsBuzzerActiveUntilDismissed() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(true);
  ClockAppController controller(alarm_service);
  controller.initialize();

  controller.refresh(true, makeTime(6, 30, 0));
  const ClockAppEffects effects = controller.refresh(true, makeTime(6, 31, 0));

  TEST_ASSERT_FALSE(effects.start_buzzer);
  TEST_ASSERT_FALSE(effects.stop_buzzer);
  TEST_ASSERT_TRUE(effects.update_buzzer);
  TEST_ASSERT_EQUAL_STRING("Dismiss", controller.state().primary_button_text);
}

void testControllerToggleEnablesAlarm() {
  AlarmService alarm_service;
  ClockAppController controller(alarm_service);
  controller.initialize();

  const ClockAppEffects effects = controller.onToggleAlarmPressed();

  TEST_ASSERT_FALSE(effects.start_buzzer);
  TEST_ASSERT_FALSE(effects.stop_buzzer);
  TEST_ASSERT_TRUE(alarm_service.isEnabled());
  TEST_ASSERT_TRUE(controller.state().alarm_enabled);
}

void testControllerToggleDisablesAlarmAndStopsBuzzer() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(6, 30, 0));

  const ClockAppEffects effects = controller.onToggleAlarmPressed();

  TEST_ASSERT_TRUE(effects.stop_buzzer);
  TEST_ASSERT_FALSE(alarm_service.isEnabled());
  TEST_ASSERT_FALSE(alarm_service.isRinging());
  TEST_ASSERT_EQUAL_STRING("06:30", controller.state().alarm_text);
  TEST_ASSERT_EQUAL_STRING("Set", controller.state().primary_button_text);
  TEST_ASSERT_FALSE(controller.state().alarm_enabled);
}

void testControllerPrimaryButtonOpensAlarmSettingsWhenIdle() {
  AlarmService alarm_service;
  alarm_service.setAlarm(8, 45);
  ClockAppController controller(alarm_service);
  controller.initialize();

  const ClockAppEffects effects = controller.onPrimaryButtonPressed();

  TEST_ASSERT_TRUE(controller.state().alarm_settings_visible);
  TEST_ASSERT_TRUE(effects.sync_alarm_settings_selection);
  TEST_ASSERT_EQUAL_UINT8(8, effects.settings_hour);
  TEST_ASSERT_EQUAL_UINT8(45, effects.settings_minute);
}

void testControllerPrimaryButtonDismissesWhenRinging() {
  AlarmService alarm_service;
  alarm_service.setAlarm(6, 30);
  alarm_service.setEnabled(true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(6, 30, 0));

  const ClockAppEffects effects = controller.onPrimaryButtonPressed();

  TEST_ASSERT_TRUE(effects.stop_buzzer);
  TEST_ASSERT_FALSE(alarm_service.isRinging());
  TEST_ASSERT_FALSE(controller.state().alarm_settings_visible);
  TEST_ASSERT_EQUAL_STRING("Set", controller.state().primary_button_text);
}

void testControllerApplyUpdatesAlarmAndHidesSettingsWithoutChangingEnabledState() {
  AlarmService alarm_service;
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.onPrimaryButtonPressed();

  const ClockAppEffects effects = controller.onApplyAlarmPressed(23, 59, AlarmService::kEveryDayMask);

  TEST_ASSERT_FALSE(effects.start_buzzer);
  TEST_ASSERT_FALSE(alarm_service.isEnabled());
  TEST_ASSERT_EQUAL_UINT8(23, alarm_service.hour());
  TEST_ASSERT_EQUAL_UINT8(59, alarm_service.minute());
  TEST_ASSERT_FALSE(controller.state().alarm_settings_visible);
  TEST_ASSERT_EQUAL_STRING("23:59", controller.state().alarm_text);
  TEST_ASSERT_EQUAL_STRING("Set", controller.state().primary_button_text);
  TEST_ASSERT_FALSE(controller.state().alarm_enabled);
}

void testControllerCancelHidesSettingsWithoutChangingAlarm() {
  AlarmService alarm_service;
  alarm_service.setAlarm(9, 10);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.onPrimaryButtonPressed();

  controller.onCancelAlarmPressed();

  TEST_ASSERT_FALSE(controller.state().alarm_settings_visible);
  TEST_ASSERT_EQUAL_UINT8(9, alarm_service.hour());
  TEST_ASSERT_EQUAL_UINT8(10, alarm_service.minute());
  TEST_ASSERT_FALSE(alarm_service.isEnabled());
}

}  // namespace

void setUp() {}

void tearDown() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;

  UNITY_BEGIN();
  RUN_TEST(testAlarmServiceDefaults);
  RUN_TEST(testAlarmServiceAddsAlarmWithRequestedTime);
  RUN_TEST(testAlarmServiceKeepsOnlyTheLatestSimultaneousAlarmRinging);
  RUN_TEST(testAlarmServiceStoresPerAlarmRingtone);
  RUN_TEST(testAlarmServiceWrapsHourAndMinute);
  RUN_TEST(testAlarmServiceDoesNotRingWhenDisabled);
  RUN_TEST(testAlarmServiceTriggersOnlyOncePerMinute);
  RUN_TEST(testAlarmServiceDismissSuppressesSameMinuteButAllowsLaterRetrigger);
  RUN_TEST(testAlarmServiceOnlyRingsOnSelectedWeekdays);
  RUN_TEST(testAlarmServiceDoesNotRingWhenNoWeekdaysAreSelected);
  RUN_TEST(testAlarmServiceFindsNextEnabledOccurrenceUsingRepeatRules);
  RUN_TEST(testAlarmServiceFindsTomorrowWhenTodaysAlarmHasPassed);
  RUN_TEST(testAlarmServiceFindsNextWeeksOnlySelectedWeekdayAfterItHasPassed);
  RUN_TEST(testAlarmServiceSnoozesRingingAlarmForTenMinutes);
  RUN_TEST(testClockFormatterFormatsTimeValues);
  RUN_TEST(testClockFormatterFormatsShortDate);
  RUN_TEST(testClockFormatterHandlesNullAndZeroBuffers);
  RUN_TEST(testClockFormatterTruncatesSafely);
  RUN_TEST(testControllerInitializeSetsDefaultViewState);
  RUN_TEST(testControllerRefreshUnavailableKeepsPlaceholderClock);
  RUN_TEST(testControllerRefreshStartsAndUpdatesBuzzerWhenAlarmTriggers);
  RUN_TEST(testControllerRefreshKeepsColonInClockText);
  RUN_TEST(testControllerLabelsTomorrowAlarmEvenWhenItIsLessThanTwentyFourHoursAway);
  RUN_TEST(testControllerOmitsTodayAndZeroHoursFromNextAlarmText);
  RUN_TEST(testControllerPrioritizesSnoozeStatusInNextAlarmText);
  RUN_TEST(testControllerRefreshKeepsBuzzerActiveUntilDismissed);
  RUN_TEST(testControllerToggleEnablesAlarm);
  RUN_TEST(testControllerToggleDisablesAlarmAndStopsBuzzer);
  RUN_TEST(testControllerPrimaryButtonOpensAlarmSettingsWhenIdle);
  RUN_TEST(testControllerPrimaryButtonDismissesWhenRinging);
  RUN_TEST(testControllerApplyUpdatesAlarmAndHidesSettingsWithoutChangingEnabledState);
  RUN_TEST(testControllerCancelHidesSettingsWithoutChangingAlarm);
  return UNITY_END();
}
