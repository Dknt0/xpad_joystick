#ifndef JOYSTICK_STATE_H
#define JOYSTICK_STATE_H

#include <linux/joystick.h>
#include <yaml-cpp/yaml.h>

#include <iomanip>
#include <iostream>
#include <mutex>

typedef struct js_event JsEvent;

template <typename T>
int sign(T value) {
  return (T(0) < value) - (value < T(0));
}

class JoystickState {
 public:
  // Axes
  double left_horizontal_;   // X Axis  left [-1, 1] right
  double left_vertical_;     // Y Axis  down [-1, 1] up
  double left_trigger_;      // Z Axis  free [0, 1] press
  double right_horizontal_;  // Rx Axis  left [-1, 1] right
  double right_vertical_;    // Ry Axis  down [-1, 1] up
  double right_trigger_;     // Rz Axis  free [0, 1] press
  double hat_horizontal_;    // Hat0X  left {-1, 0, 1} right
  double hat_vertical_;      // Hat0Y  down {-1, 0, 1} up

  // Buttons
  bool button_A_;             // A
  bool button_B_;             // B
  bool button_X_;             // X
  bool button_Y_;             // Y
  bool button_left_bumper_;   // LB
  bool button_right_bumper_;  // RB
  bool button_back_;          // Back
  bool button_start_;         // Start
  bool button_guide_;         // Guide
  bool button_left_thumb_;    // Left Thumb
  bool button_right_thumb_;   // Right Thumb

  // Input map
  std::map<u_char, double*> axis_map_;
  std::map<u_char, bool*> button_map_;

  void PrintInputMap() {
    std::cout << "Axis input map" << std::endl;
    for (const auto& itr : axis_map_) {
      std::cout << "Id: " << int(itr.first) << " Address: " << long(itr.second)
                << std::endl;
    }
    std::cout << "Button input map" << std::endl;
    for (const auto& itr : button_map_) {
      std::cout << "Id: " << int(itr.first) << " Address: " << long(itr.second)
                << std::endl;
    }
  }

  /// @brief Set user defined input map
  /// @param config
  void SetInputMap(const YAML::Node& config) {
    std::cout << "Read user defined input map from yaml file." << std::endl;
    axis_map_.clear();
    button_map_.clear();

    // Axes
    axis_map_[config["axis_map"]["X Axis"].as<u_char>()] = &left_horizontal_;
    axis_map_[config["axis_map"]["Y Axis"].as<u_char>()] = &left_vertical_;
    axis_map_[config["axis_map"]["Z Axis"].as<u_char>()] = &left_trigger_;
    axis_map_[config["axis_map"]["Rx Axis"].as<u_char>()] = &right_horizontal_;
    axis_map_[config["axis_map"]["Ry Axis"].as<u_char>()] = &right_vertical_;
    axis_map_[config["axis_map"]["Rz Axis"].as<u_char>()] = &right_trigger_;
    axis_map_[config["axis_map"]["Hat0X"].as<u_char>()] = &hat_horizontal_;
    axis_map_[config["axis_map"]["Hat0Y"].as<u_char>()] = &hat_vertical_;

    // Buttons
    button_map_[config["button_map"]["A"].as<u_char>()] = &button_A_;
    button_map_[config["button_map"]["B"].as<u_char>()] = &button_B_;
    button_map_[config["button_map"]["X"].as<u_char>()] = &button_X_;
    button_map_[config["button_map"]["Y"].as<u_char>()] = &button_Y_;
    button_map_[config["button_map"]["LB"].as<u_char>()] = &button_left_bumper_;
    button_map_[config["button_map"]["RB"].as<u_char>()] =
        &button_right_bumper_;
    button_map_[config["button_map"]["Back"].as<u_char>()] = &button_back_;
    button_map_[config["button_map"]["Start"].as<u_char>()] = &button_start_;
    button_map_[config["button_map"]["Guide"].as<u_char>()] = &button_guide_;
    button_map_[config["button_map"]["Left Thumb"].as<u_char>()] =
        &button_left_thumb_;
    button_map_[config["button_map"]["Right Thumb"].as<u_char>()] =
        &button_right_thumb_;
  }

  /// @brief Set default input map
  void SetInputMap() {
    std::cout << "Use default input map." << std::endl;
    axis_map_.clear();
    button_map_.clear();

    // Axes
    axis_map_[0] = &left_horizontal_;
    axis_map_[1] = &left_vertical_;
    axis_map_[2] = &left_trigger_;
    axis_map_[3] = &right_horizontal_;
    axis_map_[4] = &right_vertical_;
    axis_map_[5] = &right_trigger_;
    axis_map_[6] = &hat_horizontal_;
    axis_map_[7] = &hat_vertical_;

    // Buttons
    button_map_[0] = &button_A_;
    button_map_[1] = &button_B_;
    button_map_[2] = &button_X_;
    button_map_[3] = &button_Y_;
    button_map_[4] = &button_left_bumper_;
    button_map_[5] = &button_right_bumper_;
    button_map_[6] = &button_back_;
    button_map_[7] = &button_start_;
    button_map_[8] = &button_guide_;
    button_map_[9] = &button_left_thumb_;
    button_map_[10] = &button_right_thumb_;
  }

  inline void SetFromJsEvent(const JsEvent& js) {
    switch (js.type & ~JS_EVENT_INIT) {
      case JS_EVENT_BUTTON:
        // button_row_[js.number] = js.value;
        SetButtonValue(js.number, js.value);
        break;
      case JS_EVENT_AXIS:
        // axis_row_[js.number] = js.value;
        SetAxisValue(js.number, js.value);
        break;
    }
  }

  void PrintState() {
    std::unique_lock<std::mutex> lock(data_lock_);
    auto current_time = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(current_time);
    std::cout << "--- " << std::ctime(&time) << std::flush;
    std::cout << std::fixed << std::setprecision(2)
              << "Axes: X=" << left_horizontal_ << " Y=" << left_vertical_
              << " Z=" << left_trigger_ << " Rx=" << right_horizontal_
              << " Ry=" << right_vertical_ << " Rz=" << right_trigger_
              << " Hat0X=" << hat_horizontal_ << " Hat0Y=" << hat_vertical_
              << std::endl;
    std::cout << "But: A=" << button_A_ << " B=" << button_B_
              << " X=" << button_X_ << " Y=" << button_Y_
              << " LB=" << button_left_bumper_ << " RB=" << button_right_bumper_
              << " Back=" << button_back_ << " Start=" << button_start_
              << " Guide=" << button_guide_ << " LT=" << button_left_thumb_
              << " RT=" << button_right_thumb_ << std::endl;
  }

 private:
  std::mutex data_lock_;

  inline void SetAxisValue(u_char number, short value) {
    std::unique_lock<std::mutex> lock(data_lock_);
    double* axis_ptr = axis_map_[number];

    if (axis_ptr == &left_horizontal_ || axis_ptr == &right_horizontal_) {
      *axis_ptr = double(value) / 32767.0;
    } else if (axis_ptr == &left_vertical_ || axis_ptr == &right_vertical_) {
      *axis_ptr = double(-value) / 32767.0;
    } else if (axis_ptr == &left_trigger_ || axis_ptr == &right_trigger_) {
      *axis_ptr = double(value + 32767) / 65534.0;
    } else if (axis_ptr == &hat_horizontal_) {
      *axis_ptr = sign(value);
    } else if (axis_ptr == &hat_vertical_) {
      *axis_ptr = -sign(value);
    }
  }

  inline void SetButtonValue(u_char number, short value) {
    std::unique_lock<std::mutex> lock(data_lock_);
    *button_map_[number] = (value ? true : false);
  }
};

#endif
