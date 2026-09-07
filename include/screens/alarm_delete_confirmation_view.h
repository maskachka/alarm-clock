#pragma once
#include <stdint.h>
#include <lvgl.h>
enum class ConfirmationAction { DeleteAlarm };
struct ConfirmationDialogConfig {
  ConfirmationAction action;
  uint8_t alarm_index;
  const char* message;
  const char* confirm_label;
  bool destructive;
};
class ConfirmationDialogListener {
 public:
  virtual ~ConfirmationDialogListener() = default;
  virtual void onConfirmationConfirmed(ConfirmationAction action, uint8_t index) = 0;
  virtual void onConfirmationCancelled() = 0;
};
class ConfirmationDialogView {
 public:
  explicit ConfirmationDialogView(ConfirmationDialogListener& listener);
  void build(lv_obj_t* parent);
  void show(const ConfirmationDialogConfig& config);
  void hide();

 private:
  static void onConfirm(lv_event_t* event);
  static void onCancel(lv_event_t* event);
  ConfirmationDialogListener& listener_;
  lv_obj_t* overlay_;
  lv_obj_t* message_;
  lv_obj_t* confirm_button_;
  lv_obj_t* confirm_label_;
  ConfirmationDialogConfig config_;
};
