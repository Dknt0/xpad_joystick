#ifndef XBOX_JOYSTICK_H
#define XBOX_JOYSTICK_H

#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <yaml-cpp/yaml.h>

#include <chrono>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "joystick_state.h"

typedef struct js_event JsEvent;

/// @brief Rumble command
struct JoystickRumblePack {
  char cmd;  // Set to 0x03
  struct {
    u_char weak : 1;
    u_char strong : 1;
    u_char right : 1;
    u_char left : 1;
  } enable;  // 0 or 1
  struct {
    u_char left;
    u_char right;
    u_char strong;
    u_char weak;
  } strength;  // 0 ~ 100
  struct {
    u_char sustain_10ms;
    u_char release_10ms;
    u_char loop_count;  // 0 ~ 255
  } pulse;
};

class XBoxJoystick {
 public:
  XBoxJoystick(const std::string &config_path);
  ~XBoxJoystick() { close(hidraw_); }

  bool Open();
  void Close();
  bool Rumble();

  unsigned char GetAxes() { return num_axis_; }
  unsigned char GetButtons() { return num_button_; }
  int GetFd() { return fd_; }
  void PrintData();

 private:
  void Read();
  void ProcessData(const JsEvent &js);

  bool debug_;
  int fd_ = -1;                   // File descriptor used by system call
  std::string dev_path_ = "";     // Device path
  int version_ = 0x000800;        // Joystick version
  char name_[512] = "Unkown";     // Joystick name
  size_t num_axis_ = 0;           // Number of axes
  size_t num_button_ = 0;         // Number of buttons
  JoystickState joystick_state_;  // State

  std::string hid_path_ = "";          // HID path
  JoystickRumblePack rumble_command_;  // Rumble command
  int hidraw_;  // Low-level access to Human Interface Devices (HID)

  std::thread reading_thread_;  // Thread to read from device
  std::promise<void> thread_end_promise_;
  std::future<void> thread_end_future_;
};

#endif
