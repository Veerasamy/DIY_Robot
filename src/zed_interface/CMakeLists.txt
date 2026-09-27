cmake_minimum_required(VERSION 3.16)
project(zed_interface)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(cv_bridge REQUIRED)
find_package(image_transport REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(OpenCV REQUIRED)

# --- ZED SDK ---------------------------------------------------------------
# Stereolabs does not ship a CMake package config on all JetPack images, so we
# locate it defensively and fail with an actionable message instead of a wall
# of CMake link errors.
set(ZED_SDK_ROOT "/usr/local/zed" CACHE PATH "ZED SDK install root")
find_path(ZED_INCLUDE_DIR sl/Camera.hpp PATHS "${ZED_SDK_ROOT}/include")
find_library(ZED_LIBRARY NAMES sl_zed PATHS "${ZED_SDK_ROOT}/lib")
if(NOT ZED_INCLUDE_DIR OR NOT ZED_LIBRARY)
  message(FATAL_ERROR
    "ZED SDK not found under ${ZED_SDK_ROOT}. Install it from stereolabs.com "
    "matching your CUDA/L4T/JetPack version, or pass -DZED_SDK_ROOT=<path>.")
endif()

find_package(CUDAToolkit REQUIRED)

# libsl_zed.so transitively needs the NVIDIA driver's libcuda/libnvcuvid/
# libnvidia-encode, which live in the OS's standard multiarch lib dir. Some
# toolchains (e.g. conda-forge's cross-compiler, used in dev sandboxes without
# a system ROS2 install) don't search that dir by default and fail to resolve
# them at link time even though they're present -- -rpath-link (link-time-only
# hint for resolving a shared lib's OWN transitive deps, distinct from plain
# -L/-rpath) fixes this defensively if the dir exists. Harmless/no-op on
# targets (e.g. a real Jetson with a system ROS2 install) that already resolve
# these via the normal system linker.
#
# IMPORTANT ordering: a conda/RoboStack environment's OWN lib dir (e.g.
# libjpeg/libtiff/libdeflate) must be searched FIRST -- some of those libs
# share a soname with an older/ABI-incompatible system package (e.g. Ubuntu's
# libjpeg.so.8 lacks the jpeg12_* symbols conda's newer libjpeg.so.8 has), so
# listing the system multiarch dir first would make the linker pick the wrong
# one and fail with unrelated "undefined reference" errors further down the
# link. $ENV{CONDA_PREFIX} is only set when actually building inside such an
# environment, so this is a no-op elsewhere.
set(_extra_rpath_link_dirs "")
if(DEFINED ENV{CONDA_PREFIX} AND EXISTS "$ENV{CONDA_PREFIX}/lib")
  list(APPEND _extra_rpath_link_dirs "-Wl,-rpath-link,$ENV{CONDA_PREFIX}/lib")
endif()
foreach(_extra_libdir /usr/lib/x86_64-linux-gnu /usr/lib/aarch64-linux-gnu)
  if(EXISTS "${_extra_libdir}")
    list(APPEND _extra_rpath_link_dirs "-Wl,-rpath-link,${_extra_libdir}")
  endif()
endforeach()
# A conda/RoboStack toolchain's own bundled glibc can be older than what
# libsl_zed.so was built against (e.g. missing hypot/hypotf's GLIBC_2.35
# symbol version) with no same-soname fix available (see comment above) --
# defer those specific symbols to runtime instead of hard-failing the link.
# A real system-glibc target (e.g. Jetson w/ system ROS2) already matches
# and never needs this.
if(DEFINED ENV{CONDA_PREFIX})
  list(APPEND _extra_rpath_link_dirs "-Wl,--allow-shlib-undefined")
endif()

include_directories(
  include
  ${ZED_INCLUDE_DIR}
)

add_executable(zed_interface_node
  src/main.cpp
  src/zed_interface_node.cpp
)

target_link_options(zed_interface_node PRIVATE ${_extra_rpath_link_dirs})

target_link_libraries(zed_interface_node
  ${ZED_LIBRARY}
  CUDA::cudart
  ${OpenCV_LIBS}
)

target_include_directories(zed_interface_node PRIVATE ${OpenCV_INCLUDE_DIRS})
# OpenCV is deliberately omitted from ament_target_dependencies() here: some
# OpenCV CMake package configs (e.g. conda-forge/vcpkg builds) aren't
# recognized as "found" by that macro even though find_package(OpenCV)
# above succeeded -- already linked directly via target_link_libraries above.
ament_target_dependencies(zed_interface_node
  rclcpp
  sensor_msgs
  nav_msgs
  geometry_msgs
  cv_bridge
  image_transport
  tf2
  tf2_ros
  tf2_geometry_msgs
)

install(TARGETS
  zed_interface_node
  DESTINATION lib/${PROJECT_NAME}
)

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
