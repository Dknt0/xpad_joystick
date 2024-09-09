#include "xbox_joystick.h"

/// @brief Constructor
/// @param dev_name
XBoxJoystick::XBoxJoystick(const std::string &dev_path, bool debug)
    : fd_(-1), dev_path_(dev_path), debug_(debug) {}

/// @brief Open joystick device
/// @return Success flag
bool XBoxJoystick::Open() {
  if (dev_path_.length() == 0) {
    return false;
  }

  // Open joystick device
  // O_NONBLOCK open
  fd_ = open(dev_path_.c_str(), O_RDONLY | O_NONBLOCK);
  if (fd_ < 0) {
    fd_ = -1;
    std::cout << "Falied to open joytick device: " << dev_path_ << std::endl;
    return false;
  }

  // Read joystick info
  // ioctl() is used to perform device-specific input/output operations
  unsigned char num_axis_tmp;
  unsigned char num_button_tmp;
  ioctl(fd_, JSIOCGVERSION, &version_);
  ioctl(fd_, JSIOCGAXES, &num_axis_tmp);
  ioctl(fd_, JSIOCGBUTTONS, &num_button_tmp);
  ioctl(fd_, JSIOCGNAME(512), name_);

  num_axis_ = num_axis_tmp;
  num_button_ = num_button_tmp;

  std::cout << "Driver version: " << (version_ >> 16) << "."
            << ((version_ >> 8)) << "." << (version_ & 0xff) << std::endl;
  std::cout << "Joystick name: " << name_ << std::endl;
  std::cout << "Number of axes: " << num_axis_ << std::endl;
  std::cout << "Number of buttons: " << num_button_ << std::endl;

  axis_row_.resize(num_axis_);
  button_row_.resize(num_button_);

  // Start the thread
  reading_thread_ = std::thread(&XBoxJoystick::Read, this);
  thread_end_future_ = thread_end_promise_.get_future();

  return true;
}

/// @brief Close joystick device
void XBoxJoystick::Close() {
  thread_end_promise_.set_value();
  reading_thread_.join();

  if (fd_ > 0) {
    close(fd_);
    fd_ = -1;
  }
}

/// @brief Threading function
/// @param[out] js joystick event
/// @return
void XBoxJoystick::Read() {
  timeval timeout;  // Time out limit
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  fd_set rfds;  // Readiness state of files
  JsEvent js;
  int len = -1;

  while (thread_end_future_.wait_for(std::chrono::microseconds(1)) == std::future_status::timeout) {
    FD_ZERO(&rfds);
    FD_SET(fd_, &rfds);
    // select() is used to to monitor multiple file descriptors
    // Once the joystick device is readable, call Read num_button_ + num_axis_
    // times to get all values
    int ret = select(fd_ + 1, &rfds, NULL, NULL, &timeout);
    if (ret > 0 && FD_ISSET(fd_, &rfds)) {
      // ret = joystick_xbox->Read(js);
      // Reset the js_event
      memset(&js, 0, sizeof(js));
      // Read new js_event from file descriptor
      len = read(fd_, &js, sizeof(struct js_event));
      // Check size
      if (len != sizeof(struct js_event)) {
        printf("XBoxJoystick: error reading, %d(%s)\n", errno, strerror(errno));
        return;
      }

      this->ProcessData(js);
    }
  }

  return;
}

/// @brief Process one joystick event, get axise or button values
/// @param js joystick event
void XBoxJoystick::ProcessData(const JsEvent &js) {
  data_mutex_.lock();
  switch (js.type & ~JS_EVENT_INIT) {
    case JS_EVENT_BUTTON:
      button_row_[js.number] = js.value;
      break;
    case JS_EVENT_AXIS:
      axis_row_[js.number] = js.value;
      break;
  }
  data_mutex_.unlock();

  if (debug_) {
    PrintData();
  }
}

/// @brief Print current joystick states.
void XBoxJoystick::PrintData() {
  auto current_time = std::chrono::system_clock::now();
  std::time_t time = std::chrono::system_clock::to_time_t(current_time);
  std::cout << "--- " << std::ctime(&time) << std::flush;

  data_mutex_.lock();
  if (num_axis_ > 0) {
    std::cout << "Axes: ";
    for (int i = 0; i < num_axis_; ++i) {
      std::cout << i << ":" << std::setprecision(6) << axis_row_[i] << " ";
    }
  }
  std::cout << std::endl;
  if (num_button_ > 0) {
    std::cout << "Buttons: ";
    for (int i = 0; i < num_button_; ++i) {
      std::cout << i << ":" << (button_row_[i] ? "on " : "off") << " ";
    }
  }
  std::cout << std::endl;
  data_mutex_.unlock();
}
