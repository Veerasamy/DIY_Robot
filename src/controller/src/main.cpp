#include "controller/pure_pursuit_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<controller::PurePursuitNode>());
  rclcpp::shutdown();
  return 0;
}
