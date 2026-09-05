#include "ui/theme.h"

void UiTheme::applyToScreen(lv_obj_t* screen) {
  lv_obj_set_style_bg_color(screen, lv_color_hex(kSurface), 0);
  lv_obj_set_style_text_color(screen, lv_color_hex(kTextPrimary), 0);
}
