#include "xbox_joystick.h"

int main() {
  std::string dev_name = "/dev/input/js0";
  XBoxJoystick joystick_xbox(dev_name, true);
  joystick_xbox.Open();

  std::cin.get();

  joystick_xbox.Close();

  return 0;
}
