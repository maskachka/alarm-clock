#pragma once

class AlarmBuzzer {
 public:
  virtual ~AlarmBuzzer() = default;

  virtual void begin() = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void update() = 0;
};

class SilentAlarmBuzzer : public AlarmBuzzer {
 public:
  void begin() override;
  void start() override;
  void stop() override;
  void update() override;
};

#if defined(ARDUINO)
class Esp32PassiveBuzzer : public AlarmBuzzer {
 public:
  explicit Esp32PassiveBuzzer(int pin, int channel = 0);

  void begin() override;
  void start() override;
  void stop() override;
  void update() override;

 private:
  void playCurrentNote();

  int pin_;
  int channel_;
  unsigned long last_note_change_ms_;
  int note_index_;
  bool active_;
  bool initialized_;
};
#endif
