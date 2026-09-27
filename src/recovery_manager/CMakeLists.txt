cmake_minimum_required(VERSION 3.16)
project(recovery_manager)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(ackermann_msgs REQUIRED)
find_package(safety_manager REQUIRED)

include_directories(include)

add_library(dwa_planner src/dwa_planner.cpp)
target_include_directories(dwa_planner PUBLIC include)

add_executable(recovery_manager_node
  src/main.cpp
  src/recovery_manager_node.cpp
)
target_link_libraries(recovery_manager_node dwa_planner)
ament_target_dependencies(recovery_manager_node
  rclcpp
  nav_msgs
  std_msgs
  ackermann_msgs
  safety_manager
)

install(TARGETS
  recovery_manager_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS dwa_planner DESTINATION lib)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
