#include <unity.h>

void testAppSettingsDefaultsToSeventyPercentVolume();
void testAppSettingsRestoresPersistedVolume();
void testAppSettingsClampsVolumeToOneHundredPercent();
void testAlarmServiceDefaults();
void testAlarmServiceAddsAlarmWithRequestedTime();
void testAlarmServiceKeepsOnlyTheLatestSimultaneousAlarmRinging();
void testAlarmServiceStoresPerAlarmRingtone();
void testAlarmServiceRestoresSavedAlarmsFromStorage();
void testAlarmServiceWrapsHourAndMinute();
void testAlarmServiceDoesNotRingWhenDisabled();
void testAlarmServiceTriggersOnlyOncePerMinute();
void testAlarmServiceDismissSuppressesSameMinuteButAllowsLaterRetrigger();
void testAlarmServiceOnlyRingsOnSelectedWeekdays();
void testAlarmServiceDoesNotRingWhenNoWeekdaysAreSelected();
void testAlarmServiceFindsNextEnabledOccurrenceUsingRepeatRules();
void testAlarmServiceFindsTomorrowWhenTodaysAlarmHasPassed();
void testAlarmServiceFindsNextWeeksOnlySelectedWeekdayAfterItHasPassed();
void testAlarmServiceSnoozesRingingAlarmForTenMinutes();
void testClockFormatterFormatsTimeValues();
void testClockFormatterFormatsShortDate();
void testClockFormatterHandlesNullAndZeroBuffers();
void testClockFormatterTruncatesSafely();
void testControllerInitializeSetsDefaultViewState();
void testControllerRefreshUnavailableKeepsPlaceholderClock();
void testControllerRefreshStartsAndUpdatesBuzzerWhenAlarmTriggers();
void testControllerRefreshKeepsColonInClockText();
void testControllerLabelsTomorrowAlarmEvenWhenItIsLessThanTwentyFourHoursAway();
void testControllerOmitsTodayAndZeroHoursFromNextAlarmText();
void testControllerPrioritizesSnoozeStatusInNextAlarmText();
void testControllerRefreshKeepsBuzzerActiveUntilDismissed();
void testControllerToggleEnablesAlarm();
void testControllerToggleDisablesAlarmAndStopsBuzzer();
void testControllerPrimaryButtonOpensAlarmSettingsWhenIdle();
void testControllerPrimaryButtonDismissesWhenRinging();
void testControllerApplyUpdatesAlarmAndHidesSettingsWithoutChangingEnabledState();
void testControllerCancelHidesSettingsWithoutChangingAlarm();

void setUp() {}
void tearDown() {}

int main(int argc, char** argv) {
  (void)argc;
  (void)argv;
  UNITY_BEGIN();
  RUN_TEST(testAppSettingsDefaultsToSeventyPercentVolume);
  RUN_TEST(testAppSettingsRestoresPersistedVolume);
  RUN_TEST(testAppSettingsClampsVolumeToOneHundredPercent);
  RUN_TEST(testAlarmServiceDefaults);
  RUN_TEST(testAlarmServiceAddsAlarmWithRequestedTime);
  RUN_TEST(testAlarmServiceKeepsOnlyTheLatestSimultaneousAlarmRinging);
  RUN_TEST(testAlarmServiceStoresPerAlarmRingtone);
  RUN_TEST(testAlarmServiceRestoresSavedAlarmsFromStorage);
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
