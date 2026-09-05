#include "ui/button.h"

Ui::ButtonElements Ui::createButton(lv_obj_t* parent, const char* text, const ButtonStyle& style,
                                    const ButtonSize& size, lv_event_cb_t click_handler, void* event_user_data) {
  lv_obj_t* button = lv_button_create(parent);
  lv_obj_set_size(button, size.width, size.height);
  lv_obj_set_style_radius(button, style.radius, 0);
  lv_obj_set_style_bg_color(button, lv_color_hex(style.background_hex), 0);
  lv_obj_set_style_text_color(button, lv_color_hex(style.text_hex), 0);
  lv_obj_set_style_text_font(button, style.font, 0);
  lv_obj_add_event_cb(button, click_handler, LV_EVENT_CLICKED, event_user_data);

  lv_obj_t* label = lv_label_create(button);
  lv_label_set_text(label, text);
  lv_obj_set_style_text_color(label, lv_color_hex(style.text_hex), 0);
  lv_obj_set_style_text_font(label, style.font, 0);
  lv_obj_center(label);

  return {button, label};
}
