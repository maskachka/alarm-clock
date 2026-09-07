#include <unity.h>

#include "alarm_service.h"
#include "clock_app_controller.h"
#include "test_helpers.h"

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
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  const ClockAppEffects effects = controller.refresh(true, makeTime(6, 30));
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
  alarm_service.setAlarm(0, 5, 45);
  alarm_service.setWeekdayMask(0, static_cast<uint8_t>(1u << 1));
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(15, 17, 0, 0, 6, 9));
  TEST_ASSERT_EQUAL_STRING("Next alarm tomorrow at 05:45", controller.state().next_alarm_text);
}

void testControllerOmitsTodayAndZeroHoursFromNextAlarmText() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 21, 30);
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(20, 31, 0, 0, 6, 9));
  TEST_ASSERT_EQUAL_STRING("Next alarm in 59m, at 21:30", controller.state().next_alarm_text);
}

void testControllerPrioritizesSnoozeStatusInNextAlarmText() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 21, 36);
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
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(6, 30));
  const ClockAppEffects effects = controller.refresh(true, makeTime(6, 31));
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
  TEST_ASSERT_TRUE(alarm_service.isEnabled(0));
  TEST_ASSERT_TRUE(controller.state().alarm_enabled);
}

void testControllerToggleDisablesAlarmAndStopsBuzzer() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(6, 30));
  const ClockAppEffects effects = controller.onToggleAlarmPressed();
  TEST_ASSERT_TRUE(effects.stop_buzzer);
  TEST_ASSERT_FALSE(alarm_service.isEnabled(0));
  TEST_ASSERT_FALSE(alarm_service.isRinging(0));
  TEST_ASSERT_EQUAL_STRING("06:30", controller.state().alarm_text);
  TEST_ASSERT_EQUAL_STRING("Set", controller.state().primary_button_text);
  TEST_ASSERT_FALSE(controller.state().alarm_enabled);
}

void testControllerPrimaryButtonOpensAlarmSettingsWhenIdle() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 8, 45);
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
  alarm_service.setAlarm(0, 6, 30);
  alarm_service.setEnabled(0, true);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.refresh(true, makeTime(6, 30));
  const ClockAppEffects effects = controller.onPrimaryButtonPressed();
  TEST_ASSERT_TRUE(effects.stop_buzzer);
  TEST_ASSERT_FALSE(alarm_service.isRinging(0));
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
  TEST_ASSERT_FALSE(alarm_service.isEnabled(0));
  TEST_ASSERT_EQUAL_UINT8(23, alarm_service.hour(0));
  TEST_ASSERT_EQUAL_UINT8(59, alarm_service.minute(0));
  TEST_ASSERT_FALSE(controller.state().alarm_settings_visible);
  TEST_ASSERT_EQUAL_STRING("23:59", controller.state().alarm_text);
  TEST_ASSERT_EQUAL_STRING("Set", controller.state().primary_button_text);
  TEST_ASSERT_FALSE(controller.state().alarm_enabled);
}

void testControllerCancelHidesSettingsWithoutChangingAlarm() {
  AlarmService alarm_service;
  alarm_service.setAlarm(0, 9, 10);
  ClockAppController controller(alarm_service);
  controller.initialize();
  controller.onPrimaryButtonPressed();
  controller.onCancelAlarmPressed();
  TEST_ASSERT_FALSE(controller.state().alarm_settings_visible);
  TEST_ASSERT_EQUAL_UINT8(9, alarm_service.hour(0));
  TEST_ASSERT_EQUAL_UINT8(10, alarm_service.minute(0));
  TEST_ASSERT_FALSE(alarm_service.isEnabled(0));
}
