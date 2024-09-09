#include "xbox_joystick.h"

int main() {
  std::string dev_name = "/dev/input/js0";
  std::string hid_path = "/dev/hidraw2";
  XBoxJoystick joystick_xbox(dev_name, hid_path, true);
  joystick_xbox.Open();

  while (true) {

    std::cin.get();
    joystick_xbox.Rumble();
  }

  joystick_xbox.Close();

  return 0;
}
