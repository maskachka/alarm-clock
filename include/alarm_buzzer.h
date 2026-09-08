#pragma once

#include "alarm_ringtone.h"

class AlarmBuzzer {
 public:
  virtual ~AlarmBuzzer() = default;

  virtual void begin() = 0;
  virtual void setVolume(uint8_t volume) = 0;
  virtual void start(AlarmRingtone ringtone) = 0;
  virtual void stop() = 0;
  virtual void update() = 0;
};

class SilentAlarmBuzzer : public AlarmBuzzer {
 public:
  void begin() override;
  void setVolume(uint8_t volume) override;
  void start(AlarmRingtone ringtone) override;
  void stop() override;
  void update() override;
};

#if defined(ARDUINO)
class Esp32PassiveBuzzer : public AlarmBuzzer {
 public:
  explicit Esp32PassiveBuzzer(int pin, int channel = 0);

  void begin() override;
  void setVolume(uint8_t volume) override;
  void start(AlarmRingtone ringtone) override;
  void stop() override;
  void update() override;

 private:
  void playCurrentNote();

  int pin_;
  int channel_;
  unsigned long last_note_change_ms_;
  int note_index_;
  AlarmRingtone ringtone_;
  uint8_t volume_;
  bool active_;
  bool initialized_;
};
#endif
