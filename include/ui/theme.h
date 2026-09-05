#pragma once

#include <stdint.h>

#include <lvgl.h>

#include "ui/button.h"

namespace UiTheme {
constexpr uint32_t kSurface = 0xF3F0E8;
constexpr uint32_t kSurfaceRaised = 0xF7F3EA;
constexpr uint32_t kTextPrimary = 0x1F2933;
constexpr uint32_t kAccent = 0x2D8CF0;
constexpr uint32_t kActionSecondary = 0xD9E2EC;
constexpr uint32_t kTextOnAccent = 0xFFFFFF;
constexpr Ui::ButtonStyle kPrimaryButtonStyle{kAccent, kTextOnAccent, 14, &lv_font_montserrat_16};
constexpr Ui::ButtonStyle kSecondaryButtonStyle{kActionSecondary, kTextPrimary, 14, &lv_font_montserrat_16};

void applyToScreen(lv_obj_t* screen);
}  // namespace UiTheme
