#pragma once

#include <stdint.h>

#include "clock_time.h"

class ClockService {
 public:
  ClockService(const char* tz_string, const char* ntp_primary, const char* ntp_secondary = "pool.ntp.org");

  void begin(const char* wifi_ssid, const char* wifi_password, uint32_t wifi_timeout_ms = 15000);
  bool getCurrentTime(ClockTime& out_time);
  bool isSynchronized() const;

 private:
  const char* tz_string_;
  const char* ntp_primary_;
  const char* ntp_secondary_;

  bool synchronized_;
};
