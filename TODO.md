# TODO

## Architecture

- Split `ClockApp`'s 50ms refresh into focused timers: keep LVGL/touch responsive, update the buzzer only while ringing, poll clock/alarm state at a slower cadence, and render screens only when visible state changes.
