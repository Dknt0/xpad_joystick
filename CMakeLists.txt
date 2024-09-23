cmake_minimum_required(VERSION 3.10.0)
project(xbox_joystick)

find_package(Curses REQUIRED)
find_package(yaml-cpp REQUIRED)
include_directories(
  ${CURSES_INCLUDE_DIR}
  ${YAML_CPP_INCLUDE_DIR}
  ${CMAKE_SOURCE_DIR}/include
)

add_executable(hidraw src/hidraw.c)
target_link_libraries(hidraw
  ${CURSES_LIBRARIES}
)

add_executable(xbox_joystick_test src/xbox_joystick.cc src/main.cc)
target_link_libraries(xbox_joystick_test
  pthread
  ${YAML_CPP_LIBRARIES}
)
