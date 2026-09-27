#include "planner/local_planner_node.hpp"
#include "rclcpp/rclcpp.hpp"

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<planner::LocalPlannerNode>());
  rclcpp::shutdown();
  return 0;
}
