cmake_minimum_required(VERSION 3.16)
project(planner)

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
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(include)

add_library(planner_core
  src/raceline_loader.cpp
  src/path_arbitrator.cpp
)
target_include_directories(planner_core PUBLIC include)
target_link_libraries(planner_core yaml-cpp)

add_executable(local_planner_node
  src/main.cpp
  src/local_planner_node.cpp
)
target_link_libraries(local_planner_node planner_core)
ament_target_dependencies(local_planner_node
  rclcpp
  nav_msgs
  geometry_msgs
  std_msgs
)

install(TARGETS
  local_planner_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS planner_core DESTINATION lib)

install(DIRECTORY
  launch
  config
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
