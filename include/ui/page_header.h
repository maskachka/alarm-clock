#pragma once

#include "ui/button.h"

namespace Ui {
struct PageHeaderConfig {
  const char* title;
  const char* navigation_label;
  lv_event_cb_t navigation_handler;
  void* navigation_user_data;
  const char* action_label;
  const ButtonStyle* action_style;
  ButtonSize action_size;
  lv_event_cb_t action_handler;
  void* action_user_data;
};

struct PageHeaderElements {
  lv_obj_t* root;
  lv_obj_t* title;
  lv_obj_t* navigation;
  ButtonElements action;
};

PageHeaderElements createPageHeader(lv_obj_t* parent, const PageHeaderConfig& config);
}  // namespace Ui
