#include "zed_interface/zed_interface_node.hpp"

#if __has_include(<cv_bridge/cv_bridge.hpp>)
#include <cv_bridge/cv_bridge.hpp>  // Jazzy+
#else
#include <cv_bridge/cv_bridge.h>  // Humble
#endif

#include <chrono>
#include <cstring>

namespace zed_interface
{

using namespace std::chrono_literals;

namespace
{
// ZED SDK returns images as sl::Mat; wrap the underlying buffer in a cv::Mat
// without copying (zero-copy view — see ARCHITECTURE.md section on Jetson
// performance optimization).
cv::Mat slMatToCvMat(sl::Mat & input)
{
  int cv_type;
  switch (input.getDataType()) {
    case sl::MAT_TYPE::F32_C1: cv_type = CV_32FC1; break;
    case sl::MAT_TYPE::F32_C4: cv_type = CV_32FC4; break;
    case sl::MAT_TYPE::U8_C1: cv_type = CV_8UC1; break;
    case sl::MAT_TYPE::U8_C4: cv_type = CV_8UC4; break;
    default: cv_type = CV_8UC4; break;
  }
  return cv::Mat(
    static_cast<int>(input.getHeight()), static_cast<int>(input.getWidth()), cv_type,
    input.getPtr<sl::uchar1>(sl::MEM::CPU), input.getStepBytes(sl::MEM::CPU));
}

sl::RESOLUTION parseResolution(const std::string & s)
{
  if (s == "HD1080") {return sl::RESOLUTION::HD1080;}
  if (s == "HD720") {return sl::RESOLUTION::HD720;}
  if (s == "VGA") {return sl::RESOLUTION::VGA;}
  return sl::RESOLUTION::HD720;  // best latency/FOV tradeoff for racing
}

sl::DEPTH_MODE parseDepthMode(const std::string & s)
{
  if (s == "NEURAL") {return sl::DEPTH_MODE::NEURAL;}
  if (s == "ULTRA") {return sl::DEPTH_MODE::ULTRA;}
  if (s == "QUALITY") {return sl::DEPTH_MODE::QUALITY;}
  return sl::DEPTH_MODE::PERFORMANCE;  // fastest — default for Speed Race Mode
}
}  // namespace

ZedInterfaceNode::ZedInterfaceNode()
: Node("zed_interface")
{
  resolution_ = declare_parameter<std::string>("resolution", "HD720");
  camera_fps_ = declare_parameter<int>("camera_fps", 60);
  depth_mode_ = declare_parameter<std::string>("depth_mode", "PERFORMANCE");
  depth_confidence_ = static_cast<float>(declare_parameter<double>("depth_confidence", 50.0));
  depth_texture_confidence_ =
    static_cast<float>(declare_parameter<double>("depth_texture_confidence", 100.0));
  max_depth_m_ = static_cast<float>(declare_parameter<double>("max_depth_m", 10.0));
  enable_point_cloud_ = declare_parameter<bool>("enable_point_cloud", false);
  enable_positional_tracking_ = declare_parameter<bool>("enable_positional_tracking", true);
  enable_ground_plane_ = declare_parameter<bool>("enable_ground_plane", false);
  frame_id_ = declare_parameter<std::string>("frame_id", "zed2i_camera_link");
  odom_frame_id_ = declare_parameter<std::string>("odom_frame_id", "odom");

  rgb_pub_ = create_publisher<sensor_msgs::msg::Image>("/zed/rgb/image_raw", rclcpp::SensorDataQoS());
  depth_pub_ = create_publisher<sensor_msgs::msg::Image>("/zed/depth/image", rclcpp::SensorDataQoS());
  imu_pub_ = create_publisher<sensor_msgs::msg::Imu>("/zed/imu/data", rclcpp::SensorDataQoS());
  odom_pub_ = create_publisher<nav_msgs::msg::Odometry>("/zed/odom/vo", rclcpp::QoS(10));
  if (enable_point_cloud_) {
    cloud_pub_ = create_publisher<sensor_msgs::msg::PointCloud2>(
      "/zed/depth/points", rclcpp::SensorDataQoS());
  }
  tf_broadcaster_ = std::make_unique<tf2_ros::TransformBroadcaster>(*this);

  if (!openCamera()) {
    throw std::runtime_error("Failed to open ZED 2i camera");
  }

  running_ = true;
  grab_thread_ = std::thread(&ZedInterfaceNode::grabLoop, this);

  RCLCPP_INFO(
    get_logger(),
    "zed_interface started: res=%s fps=%d depth_mode=%s point_cloud=%s tracking=%s",
    resolution_.c_str(), camera_fps_, depth_mode_.c_str(),
    enable_point_cloud_ ? "on" : "off", enable_positional_tracking_ ? "on" : "off");
}

ZedInterfaceNode::~ZedInterfaceNode()
{
  running_ = false;
  if (grab_thread_.joinable()) {grab_thread_.join();}
  camera_.close();
}

bool ZedInterfaceNode::openCamera()
{
  sl::InitParameters init_params;
  init_params.camera_resolution = parseResolution(resolution_);
  init_params.camera_fps = camera_fps_;
  init_params.depth_mode = parseDepthMode(depth_mode_);
  init_params.coordinate_units = sl::UNIT::METER;
  // ROS REP-103: X forward, Y left, Z up.
  init_params.coordinate_system = sl::COORDINATE_SYSTEM::RIGHT_HANDED_Z_UP_X_FWD;
  init_params.depth_maximum_distance = max_depth_m_;
  init_params.sdk_verbose = 1;

  auto err = camera_.open(init_params);
  if (err != sl::ERROR_CODE::SUCCESS) {
    RCLCPP_ERROR(get_logger(), "sl::Camera::open failed: %s", sl::toString(err).c_str());
    return false;
  }

  if (enable_positional_tracking_) {
    sl::PositionalTrackingParameters tracking_params;
    tracking_params.enable_area_memory = false;  // Section A: disable spatial-map reuse for latency
    tracking_params.enable_imu_fusion = true;
    auto pt_err = camera_.enablePositionalTracking(tracking_params);
    if (pt_err != sl::ERROR_CODE::SUCCESS) {
      RCLCPP_WARN(get_logger(), "enablePositionalTracking failed: %s", sl::toString(pt_err).c_str());
    }
  }

  return true;
}

void ZedInterfaceNode::grabLoop()
{
  sl::RuntimeParameters runtime_params;
  runtime_params.confidence_threshold = static_cast<int>(depth_confidence_);
  runtime_params.texture_confidence_threshold = static_cast<int>(depth_texture_confidence_);
  runtime_params.enable_fill_mode = false;  // leave holes as NaN rather than hallucinate depth

  sl::Mat rgb_mat, depth_mat, cloud_mat;

  while (running_ && rclcpp::ok()) {
    if (camera_.grab(runtime_params) == sl::ERROR_CODE::SUCCESS) {
      const rclcpp::Time stamp = now();

      camera_.retrieveImage(rgb_mat, sl::VIEW::LEFT, sl::MEM::CPU);
      publishRgb(rgb_mat, stamp);

      camera_.retrieveMeasure(depth_mat, sl::MEASURE::DEPTH, sl::MEM::CPU);
      publishDepth(depth_mat, stamp);

      if (enable_point_cloud_) {
        camera_.retrieveMeasure(cloud_mat, sl::MEASURE::XYZRGBA, sl::MEM::CPU);
        publishPointCloud(cloud_mat, stamp);
      }

      publishImu(stamp);

      if (enable_positional_tracking_) {
        tracking_state_ = camera_.getPosition(camera_pose_, sl::REFERENCE_FRAME::WORLD);
        publishOdometry(stamp);
      }
    } else {
      std::this_thread::sleep_for(1ms);
    }
  }
}

void ZedInterfaceNode::publishRgb(const sl::Mat & rgb, const rclcpp::Time & stamp)
{
  cv::Mat cv_rgba = slMatToCvMat(const_cast<sl::Mat &>(rgb));
  cv::Mat cv_bgr;
  cv::cvtColor(cv_rgba, cv_bgr, cv::COLOR_BGRA2BGR);

  std_msgs::msg::Header header;
  header.stamp = stamp;
  header.frame_id = frame_id_;
  auto msg = cv_bridge::CvImage(header, "bgr8", cv_bgr).toImageMsg();
  rgb_pub_->publish(*msg);
}

void ZedInterfaceNode::publishDepth(const sl::Mat & depth, const rclcpp::Time & stamp)
{
  cv::Mat cv_depth = slMatToCvMat(const_cast<sl::Mat &>(depth));

  std_msgs::msg::Header header;
  header.stamp = stamp;
  header.frame_id = frame_id_;
  auto msg = cv_bridge::CvImage(header, "32FC1", cv_depth).toImageMsg();
  depth_pub_->publish(*msg);
}

void ZedInterfaceNode::publishPointCloud(const sl::Mat & cloud, const rclcpp::Time & stamp)
{
  const size_t width = cloud.getWidth();
  const size_t height = cloud.getHeight();

  sensor_msgs::msg::PointCloud2 msg;
  msg.header.stamp = stamp;
  msg.header.frame_id = frame_id_;
  msg.height = static_cast<uint32_t>(height);
  msg.width = static_cast<uint32_t>(width);
  msg.is_bigendian = false;
  msg.is_dense = false;
  msg.point_step = 16;  // x,y,z,rgba (float32 x4)
  msg.row_step = msg.point_step * msg.width;

  sensor_msgs::msg::PointField field;
  auto add_field = [&](const std::string & name, uint32_t offset) {
    field.name = name;
    field.offset = offset;
    field.datatype = sensor_msgs::msg::PointField::FLOAT32;
    field.count = 1;
    msg.fields.push_back(field);
  };
  add_field("x", 0);
  add_field("y", 4);
  add_field("z", 8);
  add_field("rgb", 12);

  msg.data.resize(msg.row_step * msg.height);
  std::memcpy(msg.data.data(), cloud.getPtr<sl::uchar1>(sl::MEM::CPU), msg.data.size());

  cloud_pub_->publish(msg);
}

void ZedInterfaceNode::publishImu(const rclcpp::Time & stamp)
{
  sl::SensorsData sensors_data;
  if (camera_.getSensorsData(sensors_data, sl::TIME_REFERENCE::CURRENT) != sl::ERROR_CODE::SUCCESS) {
    return;
  }
  const auto & imu = sensors_data.imu;

  sensor_msgs::msg::Imu msg;
  msg.header.stamp = stamp;
  msg.header.frame_id = frame_id_;

  msg.orientation.x = imu.pose.getOrientation().x;
  msg.orientation.y = imu.pose.getOrientation().y;
  msg.orientation.z = imu.pose.getOrientation().z;
  msg.orientation.w = imu.pose.getOrientation().w;

  msg.angular_velocity.x = imu.angular_velocity.x * (M_PI / 180.0);
  msg.angular_velocity.y = imu.angular_velocity.y * (M_PI / 180.0);
  msg.angular_velocity.z = imu.angular_velocity.z * (M_PI / 180.0);

  msg.linear_acceleration.x = imu.linear_acceleration.x;
  msg.linear_acceleration.y = imu.linear_acceleration.y;
  msg.linear_acceleration.z = imu.linear_acceleration.z;

  // ZED SDK reports per-axis covariance; leave orientation covariance at the
  // SDK default (angular_velocity_covariance) and mark unknown fields with
  // the ROS convention of -1 in [0] when not provided.
  for (int i = 0; i < 9; ++i) {
    msg.angular_velocity_covariance[i] = static_cast<double>(imu.angular_velocity_covariance.r[i]);
    msg.linear_acceleration_covariance[i] =
      static_cast<double>(imu.linear_acceleration_covariance.r[i]);
    msg.orientation_covariance[i] = static_cast<double>(imu.pose_covariance.r[i]);
  }

  imu_pub_->publish(msg);
}

void ZedInterfaceNode::publishOdometry(const rclcpp::Time & stamp)
{
  if (tracking_state_ != sl::POSITIONAL_TRACKING_STATE::OK) {
    // SEARCHING/OFF: publish nothing rather than a stale/garbage pose; the
    // EKF in sensor_fusion treats missing VO samples as "trust IMU+encoder".
    return;
  }

  const sl::Translation & t = camera_pose_.getTranslation();
  const sl::Orientation & q = camera_pose_.getOrientation();

  nav_msgs::msg::Odometry odom;
  odom.header.stamp = stamp;
  odom.header.frame_id = odom_frame_id_;
  odom.child_frame_id = frame_id_;
  odom.pose.pose.position.x = t.x;
  odom.pose.pose.position.y = t.y;
  odom.pose.pose.position.z = t.z;
  odom.pose.pose.orientation.x = q.x;
  odom.pose.pose.orientation.y = q.y;
  odom.pose.pose.orientation.z = q.z;
  odom.pose.pose.orientation.w = q.w;

  const sl::Pose & twist_pose = camera_pose_;  // ZED reports pose+confidence, not raw twist
  (void)twist_pose;
  for (int i = 0; i < 36; ++i) {
    odom.pose.covariance[i] = 0.0;
  }
  // Diagonal-only covariance from ZED's tracking confidence (0-100, higher=better).
  double conf = std::max(1, camera_pose_.pose_confidence) / 100.0;
  double sigma = 0.02 / conf;  // heuristic: 2cm at full confidence, worse otherwise
  odom.pose.covariance[0] = sigma * sigma;
  odom.pose.covariance[7] = sigma * sigma;
  odom.pose.covariance[35] = (sigma * sigma) * 4.0;  // yaw less certain than x/y

  odom_pub_->publish(odom);

  geometry_msgs::msg::TransformStamped tf_msg;
  tf_msg.header.stamp = stamp;
  tf_msg.header.frame_id = odom_frame_id_;
  tf_msg.child_frame_id = frame_id_;
  tf_msg.transform.translation.x = t.x;
  tf_msg.transform.translation.y = t.y;
  tf_msg.transform.translation.z = t.z;
  tf_msg.transform.rotation = odom.pose.pose.orientation;
  tf_broadcaster_->sendTransform(tf_msg);
}

}  // namespace zed_interface
