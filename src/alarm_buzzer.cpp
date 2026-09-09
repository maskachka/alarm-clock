#include "alarm_buzzer.h"

#include <stdio.h>

void SilentAlarmBuzzer::begin() {}
void SilentAlarmBuzzer::setVolume(uint8_t volume) {
  printf("Alarm buzzer: volume %u%% (1-10%% output range)\n", static_cast<unsigned>(volume));
}

void SilentAlarmBuzzer::start(AlarmRingtone ringtone) {
  printf("Alarm buzzer: start ringtone %u\n", static_cast<unsigned>(ringtone));
}

void SilentAlarmBuzzer::stop() { printf("Alarm buzzer: stop\n"); }

void SilentAlarmBuzzer::update() {}

#if defined(ARDUINO)
#include <Arduino.h>

namespace {
struct Tone {
  int frequency;
  unsigned long duration_ms;
};
struct Melody {
  const Tone* tones;
  size_t length;
};
// Simple, lower-register patterns work better than rapid note runs on a passive piezo buzzer.
constexpr Tone kClassicChime[] = {{523, 140}, {0, 120}, {659, 240}, {0, 1400}};
constexpr Tone kGentlePulse[] = {{440, 180}, {0, 160}, {523, 260}, {0, 1200}};
constexpr Tone kSunrise[] = {{392, 160}, {0, 90}, {440, 160}, {0, 90}, {523, 280}, {0, 1100}};
constexpr Tone kUrgent[] = {{494, 170}, {0, 120}, {587, 170}, {0, 120}, {494, 170}, {0, 900}};
constexpr Melody kMelodies[] = {{kClassicChime, sizeof(kClassicChime) / sizeof(kClassicChime[0])},
                                {kGentlePulse, sizeof(kGentlePulse) / sizeof(kGentlePulse[0])},
                                {kSunrise, sizeof(kSunrise) / sizeof(kSunrise[0])},
                                {kUrgent, sizeof(kUrgent) / sizeof(kUrgent[0])}};
const Melody& melodyFor(AlarmRingtone ringtone) {
  const uint8_t index = static_cast<uint8_t>(ringtone);
  return kMelodies[index < static_cast<uint8_t>(AlarmRingtone::Count) ? index : 0];
}
}  // namespace

Esp32PassiveBuzzer::Esp32PassiveBuzzer(int pin, int channel)
    : pin_(pin),
      channel_(channel),
      last_note_change_ms_(0),
      note_index_(0),
      ringtone_(kDefaultAlarmRingtone),
      volume_(1),
      active_(false),
      initialized_(false) {}

void Esp32PassiveBuzzer::begin() {
  if (pin_ < 0) {
    return;
  }

  ledcSetup(channel_, 2000, 8);
  ledcAttachPin(pin_, channel_);
  initialized_ = true;
}

void Esp32PassiveBuzzer::setVolume(uint8_t volume) {
  volume_ = volume < 1 ? 1 : (volume > 100 ? 100 : volume);
  if (active_) playCurrentNote();
}

void Esp32PassiveBuzzer::start(AlarmRingtone ringtone) {
  if (!initialized_) {
    begin();
  }
  if (!initialized_) {
    return;
  }

  active_ = true;
  ringtone_ = ringtone;
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
  const Melody& melody = melodyFor(ringtone_);
  if (last_note_change_ms_ != 0 && (now_ms - last_note_change_ms_) < melody.tones[note_index_].duration_ms) {
    return;
  }

  note_index_ = (note_index_ + 1) % static_cast<int>(melody.length);
  playCurrentNote();
}

void Esp32PassiveBuzzer::playCurrentNote() {
  if (!initialized_) {
    return;
  }

  const int frequency = melodyFor(ringtone_).tones[note_index_].frequency;
  ledcWriteTone(channel_, frequency);
  const uint8_t duty_percent = static_cast<uint8_t>(1 + (static_cast<uint16_t>(volume_ - 1) * 9) / 99);
  ledcWrite(channel_, frequency == 0 ? 0 : static_cast<uint32_t>(duty_percent) * 255 / 100);
  last_note_change_ms_ = millis();
}
#endif
