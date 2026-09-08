# Alarm Clock

A touch-first alarm-clock application for an ESP32, a 240×320 ILI9341 display, an XPT2046 touchscreen, and an optional passive buzzer. It is built with Arduino, PlatformIO, and LVGL 9.

The same application can also run in an LVGL/X11 desktop window for UI development.

## Features

- Current time and date synchronized with NTP on the ESP32
- Up to four alarms, each with its own enable state, time, repeat days, and ringtone
- Persisted master alarm-volume control, shared by all alarms
- Four built-in melodies: Classic Chime, Gentle Pulse, Sunrise, and Urgent
- Global ringing overlay with Dismiss and 10-minute Snooze actions
- Alarm configuration persisted in ESP32 NVS across power loss
- Touch-oriented screens for the clock, alarm list, alarm editor, deletion confirmation, and ringtone selection

## Hardware

The firmware target is an ESP32 development board connected to:

- ILI9341-compatible 240×320 TFT display, driven by TFT_eSPI
- XPT2046 resistive touchscreen
- Optional passive buzzer on GPIO 26

The checked-in pin assignments are in [include/touchscreen_configuration.h](include/touchscreen_configuration.h) and [include/buzzer_configuration.h](include/buzzer_configuration.h). Display dimensions are in [include/display_configuration.h](include/display_configuration.h).

The display is rotated 180 degrees in firmware to match the current enclosure orientation. Adjust the display and touch rotation/calibration in [src/main_esp32.cpp](src/main_esp32.cpp) when using different hardware.

## Prerequisites

- Python 3.12 or later
- An ESP32 connected by USB for the firmware target
- On Linux, PlatformIO's USB udev rules may be required for upload permissions

Install the pinned developer tools from the repository:

```sh
python -m pip install --requirement requirements-dev.txt
```

This installs PlatformIO and the exact `clang-format` version used by CI. PlatformIO downloads the declared Arduino and library dependencies on the first build.

## Build, test, and flash

Set Wi-Fi credentials before building the ESP32 firmware. They are consumed as build flags and are not stored in the repository:

```sh
export WIFI_SSID='your-network'
export WIFI_PASSWORD='your-password'
```

Build the firmware:

```sh
pio run -e mhetesp32devkit
```

Run the native unit tests:

```sh
pio test -e native_test
```

Upload to the connected device and open its serial monitor:

```sh
pio run -e mhetesp32devkit -t upload
pio device monitor -b 115200
```

If auto-detection chooses the wrong serial device, add `--upload-port /dev/ttyUSB0` to the upload command.

To build the desktop target, install X11 development headers and run:

```sh
pio run -e desktop
```

The resulting executable opens an LVGL/X11 window and uses the host's local time. It logs buzzer activity instead of playing audio.

## Architecture

The code is deliberately separated by responsibility:

- `AlarmService` owns alarm scheduling, recurrence rules, and snooze state. `AlarmStorage` provides persistence, with an ESP32 NVS implementation and an in-memory implementation for tests and desktop development.
- `AppSettingsService` owns global preferences such as alarm volume, persisted separately from individual alarms.
- `ClockService` provides the current time, via Wi-Fi/NTP on hardware or the host clock on desktop.
- `ClockAppController` turns service state and user actions into display state and buzzer effects.
- `ClockApp` coordinates LVGL, screens, overlays, and the buzzer.
- `screens/` contains page and overlay views; `ui/` contains reusable theme, button, header, and icon primitives.

Native unit tests cover scheduling, recurrence, snooze behavior, formatting, and controller behavior. They live in `test/test_native/`.

## Configuration notes

- New alarms default to every day and the Classic Chime ringtone.
- Set `BUZZER_PIN` to `-1` in `include/buzzer_configuration.h` to disable buzzer output.
- The NTP timezone currently targets Pacific Time and is configured in `src/main_esp32.cpp`.
- Existing persisted alarm records are migrated when the storage format changes.

## Development

Formatting is defined by `.clang-format`:

```sh
clang-format -i path/to/file.cpp
```

## Continuous integration

GitHub Actions runs on every push and pull request. It verifies that all tracked C++ source and header files match `.clang-format`, then runs the native test suite.

Run the same checks locally after installing `requirements-dev.txt`:

```sh
git ls-files -z -- '*.cpp' '*.h' | xargs -0 clang-format --dry-run --Werror
pio test -e native_test
```

See [TODO.md](TODO.md) for known architectural follow-up work.

## License

This project is licensed under the [MIT License](LICENSE).
