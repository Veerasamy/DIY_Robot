# generated from rosidl_cmake/cmake/rosidl_cmake_aggregate_target-extras.cmake.in

# Create a convenience aggregate target safety_manager::safety_manager
# that links all generated interface targets, so downstream packages can use
# a single modern CMake target name instead of ${safety_manager_TARGETS}.
if(safety_manager_TARGETS AND NOT TARGET safety_manager::safety_manager)
  add_library(safety_manager::safety_manager INTERFACE IMPORTED)
  set_target_properties(safety_manager::safety_manager PROPERTIES
    INTERFACE_LINK_LIBRARIES "${safety_manager_TARGETS}")
endif()
