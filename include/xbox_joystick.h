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

#include <chrono>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <future>

typedef struct js_event JsEvent;

class XBoxJoystick {
 public:
  XBoxJoystick(const std::string &dev_path, bool debug = false);
  ~XBoxJoystick() {}
  bool Open();
  void Close();
  void Read();

  unsigned char GetAxes() { return num_axis_; }
  unsigned char GetButtons() { return num_button_; }
  int GetFd() { return fd_; }
  void PrintData();
  void ProcessData(const JsEvent &js);

 private:
  bool debug_;
  int fd_ = -1;                   // File descriptor used by system call
  std::string dev_path_ = "";     // Device path
  int version_ = 0x000800;        // Joystick version
  char name_[512] = "Unkown";     // Joystick name
  size_t num_axis_ = 0;           // Number of axes
  size_t num_button_ = 0;         // Number of buttons
  std::vector<int> axis_row_;     // Axis row value
  std::vector<char> button_row_;  // Button row value

  std::thread reading_thread_;  // Thread to read from device
  std::mutex data_mutex_;
  std::promise<void> thread_end_promise_;
  std::future<void> thread_end_future_;
};

#endif
