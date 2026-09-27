cmake_minimum_required(VERSION 3.16)
project(obstacle_detection)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)  # std::clamp (C++17) is used in vfh_planner
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(OpenCV REQUIRED)

include_directories(include)

add_library(vfh_planner src/vfh_planner.cpp)
target_include_directories(vfh_planner PUBLIC include)

add_executable(obstacle_avoidance_node
  src/main.cpp
  src/obstacle_avoidance_node.cpp
)
target_link_libraries(obstacle_avoidance_node vfh_planner ${OpenCV_LIBS})
target_include_directories(obstacle_avoidance_node PRIVATE ${OpenCV_INCLUDE_DIRS})
# OpenCV is deliberately omitted from ament_target_dependencies() here: some
# OpenCV CMake package configs (e.g. conda-forge/vcpkg builds) aren't
# recognized as "found" by that macro even though find_package(OpenCV)
# above succeeded -- already linked directly via target_link_libraries above.
ament_target_dependencies(obstacle_avoidance_node
  rclcpp
  sensor_msgs
  nav_msgs
  geometry_msgs
  std_msgs
  cv_bridge
)

install(TARGETS
  obstacle_avoidance_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS vfh_planner DESTINATION lib)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
