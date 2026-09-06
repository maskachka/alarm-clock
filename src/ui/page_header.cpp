#include "ui/page_header.h"

#include "ui/theme.h"

namespace Ui {
PageHeaderElements createPageHeader(lv_obj_t* parent, const PageHeaderConfig& config) {
  lv_obj_t* root = lv_obj_create(parent);
  lv_obj_remove_style_all(root);
  lv_obj_set_size(root, LV_PCT(100), 38);

  lv_obj_t* title = lv_label_create(root);
  lv_label_set_text(title, config.title);
  lv_obj_set_style_text_font(title, &lv_font_montserrat_20, 0);
  lv_obj_set_style_text_color(title, lv_color_hex(UiTheme::kTextPrimary), 0);
  lv_obj_align(title, LV_ALIGN_CENTER, 0, 0);

  lv_obj_t* navigation = nullptr;
  if (config.navigation_label != nullptr) {
    navigation = lv_label_create(root);
    lv_label_set_text(navigation, config.navigation_label);
    lv_obj_set_style_text_font(navigation, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(navigation, lv_color_hex(UiTheme::kAccent), 0);
    lv_obj_add_flag(navigation, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(navigation, config.navigation_handler, LV_EVENT_CLICKED, config.navigation_user_data);
    lv_obj_align(navigation, LV_ALIGN_LEFT_MID, 0, 0);
  }

  ButtonElements action{nullptr, nullptr};
  if (config.action_label != nullptr) {
    action = createButton(root, config.action_label, *config.action_style, config.action_size, config.action_handler,
                          config.action_user_data);
    lv_obj_align(action.button, LV_ALIGN_RIGHT_MID, 0, 0);
  }

  return {root, title, navigation, action};
}
}  // namespace Ui
