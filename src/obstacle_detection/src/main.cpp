#include "obstacle_detection/obstacle_avoidance_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<obstacle_detection::ObstacleAvoidanceNode>());
  rclcpp::shutdown();
  return 0;
}
