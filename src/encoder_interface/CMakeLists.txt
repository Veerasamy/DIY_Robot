cmake_minimum_required(VERSION 3.16)
project(encoder_interface)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

option(ENCODER_INTERFACE_WITH_GPIOD "Use libgpiod for GPIO quadrature decode when available" ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)

include_directories(include)

set(ENCODER_SOURCES src/encoder_node.cpp)
set(ENCODER_LIBS "")

if(ENCODER_INTERFACE_WITH_GPIOD)
  find_package(PkgConfig QUIET)
  if(PkgConfig_FOUND)
    pkg_check_modules(GPIOD QUIET libgpiod)
  endif()
  if(GPIOD_FOUND)
    message(STATUS "encoder_interface: building with libgpiod GPIO quadrature backend")
    list(APPEND ENCODER_SOURCES src/gpio_quadrature_reader.cpp)
    list(APPEND ENCODER_LIBS ${GPIOD_LIBRARIES})
  else()
    message(STATUS
      "encoder_interface: libgpiod-dev not found (sudo apt install libgpiod-dev) — "
      "building with a zero-output fallback reader")
  endif()
endif()

add_executable(encoder_node
  src/main.cpp
  ${ENCODER_SOURCES}
)
target_link_libraries(encoder_node ${ENCODER_LIBS})
if(GPIOD_FOUND)
  target_include_directories(encoder_node PRIVATE ${GPIOD_INCLUDE_DIRS})
  target_compile_definitions(encoder_node PRIVATE ENCODER_INTERFACE_HAVE_LIBGPIOD=1)
endif()
ament_target_dependencies(encoder_node
  rclcpp
  std_msgs
)

install(TARGETS
  encoder_node
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
