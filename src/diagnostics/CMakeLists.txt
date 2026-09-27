cmake_minimum_required(VERSION 3.16)
project(diagnostics)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(diagnostic_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)
find_package(safety_manager REQUIRED)

include_directories(include)

add_library(gpu_cpu_monitor src/gpu_cpu_monitor.cpp)
target_include_directories(gpu_cpu_monitor PUBLIC include)

add_executable(system_health_node
  src/main.cpp
  src/system_health_node.cpp
)
target_link_libraries(system_health_node gpu_cpu_monitor)
ament_target_dependencies(system_health_node
  rclcpp
  diagnostic_msgs
  nav_msgs
  ackermann_msgs
  safety_manager
)

install(TARGETS
  system_health_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS gpu_cpu_monitor DESTINATION lib)

install(PROGRAMS
  scripts/record_bag.py
  scripts/clear_estop.py
  scripts/gpu_profile.py
  DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
