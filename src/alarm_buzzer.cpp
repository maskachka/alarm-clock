#include "alarm_buzzer.h"

#include <stdio.h>

void SilentAlarmBuzzer::begin() {}
void SilentAlarmBuzzer::setVolume(uint8_t volume) {
  printf("Alarm buzzer: volume %u%%\n", static_cast<unsigned>(volume));
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
constexpr Tone kClassicChime[] = {{523, 180}, {659, 180}, {784, 180}, {1047, 260}};
constexpr Tone kGentlePulse[] = {{523, 320}, {0, 160}, {659, 320}, {0, 500}};
constexpr Tone kSunrise[] = {{392, 150}, {523, 150}, {659, 150}, {784, 150}, {1047, 360}};
constexpr Tone kUrgent[] = {{880, 100}, {523, 100}, {880, 100}, {523, 100}, {0, 120}};
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
      volume_(70),
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
  volume_ = volume > 100 ? 100 : volume;
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
  ledcWrite(channel_, frequency == 0 ? 0 : static_cast<uint32_t>(volume_) * 255 / 100);
  last_note_change_ms_ = millis();
}
#endif
