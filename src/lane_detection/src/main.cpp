#include "lane_detection/lane_detection_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<lane_detection::LaneDetectionNode>());
  rclcpp::shutdown();
  return 0;
}
