#include "alarm_buzzer.h"

#include <stdio.h>

void SilentAlarmBuzzer::begin() {}

void SilentAlarmBuzzer::start() {
  printf("Alarm buzzer: start\n");
}

void SilentAlarmBuzzer::stop() {
  printf("Alarm buzzer: stop\n");
}

void SilentAlarmBuzzer::update() {}

#if defined(ARDUINO)
#include <Arduino.h>

namespace {
constexpr unsigned long kNoteDurationMs = 180;
constexpr int kMelody[] = {523, 659, 784, 1047};
constexpr size_t kMelodyLength = sizeof(kMelody) / sizeof(kMelody[0]);
} // namespace

Esp32PassiveBuzzer::Esp32PassiveBuzzer(int pin, int channel)
    : pin_(pin), channel_(channel), last_note_change_ms_(0), note_index_(0), active_(false), initialized_(false) {}

void Esp32PassiveBuzzer::begin() {
  if (pin_ < 0) {
    return;
  }

  ledcSetup(channel_, 2000, 8);
  ledcAttachPin(pin_, channel_);
  initialized_ = true;
}

void Esp32PassiveBuzzer::start() {
  if (!initialized_) {
    begin();
  }
  if (!initialized_) {
    return;
  }

  active_ = true;
  note_index_ = 0;
  last_note_change_ms_ = 0;
  playCurrentNote();
}

void Esp32PassiveBuzzer::stop() {
  if (initialized_) {
    ledcWriteTone(channel_, 0);
  }
  active_ = false;
  note_index_ = 0;
}

void Esp32PassiveBuzzer::update() {
  if (!active_ || !initialized_) {
    return;
  }

  const unsigned long now_ms = millis();
  if (last_note_change_ms_ != 0 && (now_ms - last_note_change_ms_) < kNoteDurationMs) {
    return;
  }

  note_index_ = (note_index_ + 1) % static_cast<int>(kMelodyLength);
  playCurrentNote();
}

void Esp32PassiveBuzzer::playCurrentNote() {
  if (!initialized_) {
    return;
  }

  ledcWriteTone(channel_, kMelody[note_index_]);
  last_note_change_ms_ = millis();
}
#endif
