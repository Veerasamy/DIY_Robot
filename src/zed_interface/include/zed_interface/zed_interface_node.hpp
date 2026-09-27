// ZED 2i interface node — publishes depth, RGB, IMU and visual odometry.
//
// Design decision: a dedicated grab thread runs the blocking
// sl::Camera::grab() loop at the camera's native frame rate, decoupled from
// the ROS2 executor thread. This avoids the executor's timer jitter coupling
// into camera exposure/grab timing, which matters at race speed where every
// millisecond of latency shows up as steering lag.
#pragma once

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <sl/Camera.hpp>

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "nav_msgs/msg/odometry.hpp"
#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"
#include "sensor_msgs/msg/imu.hpp"
#include "sensor_msgs/msg/point_cloud2.hpp"
#include "tf2_ros/transform_broadcaster.h"

namespace zed_interface
{

class ZedInterfaceNode : public rclcpp::Node
{
public:
  ZedInterfaceNode();
  ~ZedInterfaceNode() override;

private:
  bool openCamera();
  void grabLoop();
  void publishRgb(const sl::Mat & rgb, const rclcpp::Time & stamp);
  void publishDepth(const sl::Mat & depth, const rclcpp::Time & stamp);
  void publishPointCloud(const sl::Mat & cloud, const rclcpp::Time & stamp);
  void publishImu(const rclcpp::Time & stamp);
  void publishOdometry(const rclcpp::Time & stamp);

  sl::Camera camera_;
  std::thread grab_thread_;
  std::atomic<bool> running_{false};

  // Parameters (Speed Race Mode vs Obstacle Race Mode feature toggles —
  // see ARCHITECTURE.md section 3).
  std::string resolution_;
  int camera_fps_{60};
  std::string depth_mode_;
  float depth_confidence_{50.0f};
  float depth_texture_confidence_{100.0f};
  float max_depth_m_{10.0f};
  bool enable_point_cloud_{false};
  bool enable_positional_tracking_{true};
  bool enable_ground_plane_{false};
  std::string frame_id_{"zed2i_camera_link"};
  std::string odom_frame_id_{"odom"};

  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr rgb_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr depth_pub_;
  rclcpp::Publisher<sensor_msgs::msg::Imu>::SharedPtr imu_pub_;
  rclcpp::Publisher<nav_msgs::msg::Odometry>::SharedPtr odom_pub_;
  rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr cloud_pub_;
  std::unique_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster_;

  sl::Pose camera_pose_;
  sl::POSITIONAL_TRACKING_STATE tracking_state_{sl::POSITIONAL_TRACKING_STATE::OFF};
};

}  // namespace zed_interface
