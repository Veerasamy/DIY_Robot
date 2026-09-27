cmake_minimum_required(VERSION 3.16)
project(controller)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)  # std::clamp (C++17) is used throughout this package
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)
find_package(safety_manager REQUIRED)

include_directories(include)

add_library(steering_controllers
  src/pure_pursuit_controller.cpp
  src/stanley_controller.cpp
  src/mpc_controller.cpp
)
target_include_directories(steering_controllers PUBLIC include)

add_executable(pure_pursuit_node
  src/main.cpp
  src/pure_pursuit_node.cpp
)
target_link_libraries(pure_pursuit_node steering_controllers)
ament_target_dependencies(pure_pursuit_node
  rclcpp
  nav_msgs
  std_msgs
  geometry_msgs
  ackermann_msgs
  safety_manager
)

install(TARGETS
  pure_pursuit_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS steering_controllers DESTINATION lib)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
