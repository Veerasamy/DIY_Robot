#include "rclcpp/rclcpp.hpp"
#include "safety_manager/estop_bridge_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<safety_manager::EStopBridgeNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
