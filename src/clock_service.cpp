#include "clock_service.h"

#include <string.h>

#if defined(ARDUINO)
#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#endif

ClockService::ClockService(const char* tz_string, const char* ntp_primary, const char* ntp_secondary)
    : tz_string_(tz_string), ntp_primary_(ntp_primary), ntp_secondary_(ntp_secondary), synchronized_(false) {}

void ClockService::begin(const char* wifi_ssid, const char* wifi_password, uint32_t wifi_timeout_ms) {
#if defined(ARDUINO)
  if (wifi_ssid == nullptr || strlen(wifi_ssid) == 0) {
    Serial.println("ClockService: WIFI_SSID is empty. Skipping Wi-Fi/NTP setup.");
    synchronized_ = false;
    return;
  }

  Serial.printf("ClockService: Connecting to Wi-Fi SSID '%s'...\n", wifi_ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_ssid, wifi_password);

  const uint32_t start_ms = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start_ms) < wifi_timeout_ms) {
    delay(250);
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("ClockService: Wi-Fi connect failed (status=%d).\n", static_cast<int>(WiFi.status()));
    synchronized_ = false;
    return;
  }

  Serial.print("ClockService: Wi-Fi connected. IP=");
  Serial.println(WiFi.localIP());

  configTzTime(tz_string_, ntp_primary_, ntp_secondary_);
  Serial.printf("ClockService: NTP configured (tz='%s', ntp='%s').\n", tz_string_, ntp_primary_);

  struct tm time_info;
  synchronized_ = getLocalTime(&time_info, 10000);
  if (synchronized_) {
    Serial.println("ClockService: Initial NTP sync complete.");
  } else {
    Serial.println("ClockService: Initial NTP sync timed out; will keep trying.");
  }
#else
  (void)wifi_ssid;
  (void)wifi_password;
  (void)wifi_timeout_ms;
  (void)ntp_primary_;
  (void)ntp_secondary_;

  setenv("TZ", tz_string_, 1);
  tzset();
  synchronized_ = true;
  printf("ClockService: host clock initialized (tz='%s').\n", tz_string_);
#endif
}

bool ClockService::getCurrentTime(ClockTime& out_time) {
#if defined(ARDUINO)
  struct tm local_time;
  if (!getLocalTime(&local_time, 50)) {
    return false;
  }

  synchronized_ = true;

  out_time.hour = static_cast<uint8_t>(local_time.tm_hour);
  out_time.minute = static_cast<uint8_t>(local_time.tm_min);
  out_time.second = static_cast<uint8_t>(local_time.tm_sec);
  return true;
#else
  time_t now = time(nullptr);
  if (now == static_cast<time_t>(-1)) {
    synchronized_ = false;
    return false;
  }

  struct tm local_time;
  if (localtime_r(&now, &local_time) == nullptr) {
    synchronized_ = false;
    return false;
  }

  synchronized_ = true;
  out_time.hour = static_cast<uint8_t>(local_time.tm_hour);
  out_time.minute = static_cast<uint8_t>(local_time.tm_min);
  out_time.second = static_cast<uint8_t>(local_time.tm_sec);
  return true;
#endif
}

bool ClockService::isSynchronized() const { return synchronized_; }
