cmake_minimum_required(VERSION 3.16)
project(lane_detection)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

option(LANE_DETECTION_WITH_CUDA "Use cv::cuda accelerated ops when available" ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Threads REQUIRED)

add_library(lane_detector src/lane_detector.cpp)
target_include_directories(lane_detector PUBLIC include ${OpenCV_INCLUDE_DIRS})
target_link_libraries(lane_detector ${OpenCV_LIBS} Threads::Threads)

if(LANE_DETECTION_WITH_CUDA)
  find_package(OpenCV COMPONENTS cudaarithm cudaimgproc cudawarping QUIET)
  if(OpenCV_cudaarithm_FOUND OR TARGET opencv_cudaarithm)
    message(STATUS "lane_detection: building with OpenCV CUDA acceleration")
    target_compile_definitions(lane_detector PUBLIC LANE_DETECTION_USE_CUDA=1)
  else()
    message(STATUS "lane_detection: OpenCV CUDA modules not found, building CPU-only path")
  endif()
endif()

add_executable(lane_detection_node
  src/main.cpp
  src/lane_detection_node.cpp
)
target_include_directories(lane_detection_node PRIVATE include)
target_link_libraries(lane_detection_node lane_detector)
# OpenCV is deliberately omitted here: ament_target_dependencies() doesn't
# recognize some OpenCV CMake package configs (e.g. conda-forge/vcpkg builds)
# as "found" even though find_package(OpenCV) above succeeded -- OpenCV's
# include dirs/libs already reach this target transitively via the PUBLIC
# target_include_directories/target_link_libraries on the lane_detector
# library above, so no separate include/link step is needed here.
ament_target_dependencies(lane_detection_node
  rclcpp
  sensor_msgs
  nav_msgs
  geometry_msgs
  std_msgs
  cv_bridge
  image_transport
)

install(TARGETS
  lane_detection_node
  DESTINATION lib/${PROJECT_NAME}
)
install(TARGETS lane_detector DESTINATION lib)

install(DIRECTORY
  launch
  DESTINATION share/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
