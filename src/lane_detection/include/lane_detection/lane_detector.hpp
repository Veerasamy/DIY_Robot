// Lane / road-edge detection pipeline: threshold -> bird's-eye warp ->
// sliding-window pixel extraction -> 2nd-order polynomial fit -> curvature +
// centerline. Pure algorithm class, no ROS dependency, so it is unit
// testable and reusable outside the ROS2 node.
#pragma once

#include <array>
#include <opencv2/opencv.hpp>
#include <vector>

namespace lane_detection
{

// 2nd-order polynomial in the bird's-eye (warped) image frame: x = a*y^2 + b*y + c
struct LaneFit
{
  bool valid{false};
  double a{0.0};
  double b{0.0};
  double c{0.0};

  double xAt(double y) const {return a * y * y + b * y + c;}
};

struct LaneResult
{
  LaneFit left;
  LaneFit right;
  std::vector<cv::Point2d> centerline_m;  // (x_forward, y_lateral) in vehicle frame, meters
  double curvature_radius_m{0.0};         // radius of the fitted centerline near the vehicle
  double center_offset_m{0.0};            // + = vehicle is left of centerline
  cv::Mat debug_image;                    // birds-eye view with overlays, only if requested
};

struct LaneDetectorParams
{
  // Perspective transform: four source points in the raw camera image
  // (pixels) mapped to a rectangle in the bird's-eye image. Defaults assume
  // a 1280x720 ZED left-image crop with the 2.1mm wide lens looking ~15
  // degrees down at the track — recalibrate per mounting via the
  // calibrate_perspective.py helper described in the package README.
  std::array<cv::Point2f, 4> src_points{
    cv::Point2f{200, 700}, cv::Point2f{1100, 700},
    cv::Point2f{750, 450}, cv::Point2f{530, 450}};
  cv::Size warp_size{700, 900};

  // Real course path width -- DIY Robotics Challenge "Speed Course" spec is
  // a 36in wide path (0.9144m); update via lane_detection_node's
  // lane_width_m parameter if racing a differently-specified course. Used
  // both as the meters-per-pixel_x default derivation below and as the
  // road-edge-only centerline fallback half-width.
  double lane_width_m{0.9144};

  // Meters-per-pixel calibration in the warped image, used to convert the
  // polynomial fit (pixels) into a real-world curvature radius and
  // centerline for the controller. meters_per_pixel_x's default assumes the
  // calibrated src_points bottom edge exactly bounds one lane_width_m across
  // the dst rectangle's width (0.6 * warp_size.width, see
  // computeTransforms()) -- meters_per_pixel_y still needs real on-track
  // measurement (see README; its forward-distance assumption is unverified
  // for this course, unlike the width which is now tied to a known spec).
  double meters_per_pixel_x{0.9144 / (0.6 * 700.0)};
  double meters_per_pixel_y{9.0 / 900.0};

  int sobel_thresh_min{20};
  int sobel_thresh_max{100};
  int s_channel_thresh_min{100};
  int s_channel_thresh_max{255};

  int n_windows{9};
  int window_margin{80};
  int min_pixels_to_recenter{50};

  bool generate_debug_image{false};
};

class LaneDetector
{
public:
  explicit LaneDetector(const LaneDetectorParams & params);

  // Runs the full pipeline on a BGR frame. Thread-safe to call repeatedly
  // from a single node thread; internal left/right window searches are
  // parallelized with std::async.
  LaneResult process(const cv::Mat & bgr_frame);

  void setParams(const LaneDetectorParams & params) {params_ = params; computeTransforms();}

private:
  void computeTransforms();
  cv::Mat threshold(const cv::Mat & bgr_frame) const;
  cv::Mat warp(const cv::Mat & binary) const;
  LaneFit slidingWindowSearch(const cv::Mat & binary_warped, bool search_left_half) const;
  std::vector<cv::Point2d> buildCenterline(const LaneFit & left, const LaneFit & right) const;
  double estimateCurvatureRadius(const LaneFit & fit_px, double eval_y_px) const;

  LaneDetectorParams params_;
  cv::Mat perspective_M_;
  cv::Mat perspective_Minv_;
};

}  // namespace lane_detection
