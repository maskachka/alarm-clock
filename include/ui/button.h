#pragma once

#include <stdint.h>

#include <lvgl.h>

namespace Ui {
struct ButtonStyle {
  uint32_t background_hex;
  uint32_t text_hex;
  int32_t radius;
  const lv_font_t* font;
};

struct ButtonSize {
  int32_t width;
  int32_t height;
};

struct ButtonElements {
  lv_obj_t* button;
  lv_obj_t* label;
};

ButtonElements createButton(lv_obj_t* parent, const char* text, const ButtonStyle& style, const ButtonSize& size,
                            lv_event_cb_t click_handler, void* event_user_data);
}  // namespace Ui
