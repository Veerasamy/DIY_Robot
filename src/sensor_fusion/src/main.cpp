#include "rclcpp/rclcpp.hpp"
#include "sensor_fusion/ekf_node.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<sensor_fusion::EkfNode>());
  rclcpp::shutdown();
  return 0;
}
