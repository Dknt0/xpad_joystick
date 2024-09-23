#include "xbox_joystick.h"

/// @brief Constructor
/// @param dev_name
XBoxJoystick::XBoxJoystick(const std::string &config_path) : fd_(-1) {
  // Read parameters from yaml file
  YAML::Node yaml_config = YAML::LoadFile(config_path);

  fd_ = -1;
  dev_path_ = yaml_config["dev_path"].as<std::string>();
  hid_path_ = yaml_config["hid_path"].as<std::string>();
  debug_ = yaml_config["debug"].as<bool>();

  if (yaml_config["use_user_mapping"].as<bool>()) {
    joystick_state_.SetInputMap(yaml_config);
  } else {
    joystick_state_.SetInputMap();
  }

  // joystick_state_.PrintInputMap();

  // Initialize Rumble command struct
  rumble_command_.cmd = 0x03;
  rumble_command_.enable.strong = 1;
  rumble_command_.enable.weak = 1;
  rumble_command_.enable.left = 1;
  rumble_command_.enable.right = 1;
  rumble_command_.strength.strong = 40;
  rumble_command_.strength.weak = 30;
  rumble_command_.strength.left = 20;
  rumble_command_.strength.right = 20;
  rumble_command_.pulse.sustain_10ms = 5;
  rumble_command_.pulse.release_10ms = 5;
  rumble_command_.pulse.loop_count = 1;
}

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

  // Get HID handle
  hidraw_ = open(hid_path_.c_str(), O_WRONLY);
  if (hidraw_ < 0) {
    std::cerr << "Error opening " << hid_path_ << ". Changing permission."
              << std::endl;
    system((std::string("sudo chmod 666 ") + hid_path_).c_str());
    hidraw_ = open(hid_path_.c_str(), O_WRONLY);

    if (hidraw_ < 0) {
      std::cerr << "Error hid path." << std::endl;
      return false;
    }
  }

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

  while (thread_end_future_.wait_for(std::chrono::microseconds(1)) ==
         std::future_status::timeout) {
    FD_ZERO(&rfds);
    FD_SET(fd_, &rfds);
    // select() is used to to monitor multiple file descriptors
    int ret = select(fd_ + 1, &rfds, NULL, NULL, &timeout);
    if (ret > 0 && FD_ISSET(fd_, &rfds)) {
      // ret = joystick_xbox->Read(js);
      // Reset the js_event
      memset(&js, 0, sizeof(js));
      // Read new js_event from file descriptor
      len = read(fd_, &js, sizeof(JsEvent));
      // Check size
      if (len != sizeof(JsEvent)) {
        std::cout << "XBoxJoystick error readings" << std::endl;
        return;
      }

      // this->ProcessData(js);
      joystick_state_.SetFromJsEvent(js);
      if (debug_) {
        joystick_state_.PrintState();
      }
    }
  }

  return;
}

bool XBoxJoystick::Rumble() {
  write(hidraw_, &rumble_command_, sizeof(rumble_command_));
  return true;
}
