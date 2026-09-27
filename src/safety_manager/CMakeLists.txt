cmake_minimum_required(VERSION 3.16)
project(safety_manager)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

rosidl_generate_interfaces(${PROJECT_NAME}
  "msg/EStopStatus.msg"
  "msg/VehicleState.msg"
  DEPENDENCIES builtin_interfaces std_msgs
)

include_directories(
  include
  common
)

add_executable(estop_bridge_node
  src/main.cpp
  src/estop_bridge_node.cpp
  src/vehicle_state_machine.cpp
)

ament_target_dependencies(estop_bridge_node
  rclcpp
  std_msgs
  ackermann_msgs
)

# Link the locally generated rosidl typesupport (EStopStatus/VehicleState)
rosidl_get_typesupport_target(cpp_typesupport_target ${PROJECT_NAME} "rosidl_typesupport_cpp")
target_link_libraries(estop_bridge_node "${cpp_typesupport_target}")

install(TARGETS
  estop_bridge_node
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  firmware
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
