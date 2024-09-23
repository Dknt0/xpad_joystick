#include "xbox_joystick.h"

int main() {
  XBoxJoystick joystick_xbox(
      "/home/dknt/Projects/xpad_joystick/config/default.yaml");
  joystick_xbox.Open();

  while (true) {
    std::cin.get();
    joystick_xbox.Rumble();
  }

  joystick_xbox.Close();

  return 0;
}
