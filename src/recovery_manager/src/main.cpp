#include "recovery_manager/recovery_manager_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<recovery_manager::RecoveryManagerNode>());
  rclcpp::shutdown();
  return 0;
}
