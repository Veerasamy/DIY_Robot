#include "lane_detection/lane_detector.hpp"

#include <future>

#ifdef LANE_DETECTION_USE_CUDA
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>
#endif

namespace lane_detection
{

namespace
{
// Least-squares fit of x = a*y^2 + b*y + c through (y_i, x_i) using the
// normal equations solved with cv::solve (Cholesky/SVD under the hood).
LaneFit fitPoly2(const std::vector<double> & ys, const std::vector<double> & xs)
{
  LaneFit fit;
  if (ys.size() < 3) {return fit;}

  cv::Mat A(static_cast<int>(ys.size()), 3, CV_64F);
  cv::Mat b(static_cast<int>(ys.size()), 1, CV_64F);
  for (size_t i = 0; i < ys.size(); ++i) {
    double y = ys[i];
    A.at<double>(static_cast<int>(i), 0) = y * y;
    A.at<double>(static_cast<int>(i), 1) = y;
    A.at<double>(static_cast<int>(i), 2) = 1.0;
    b.at<double>(static_cast<int>(i), 0) = xs[i];
  }

  cv::Mat coeffs;
  if (!cv::solve(A, b, coeffs, cv::DECOMP_SVD)) {return fit;}

  fit.a = coeffs.at<double>(0);
  fit.b = coeffs.at<double>(1);
  fit.c = coeffs.at<double>(2);
  fit.valid = true;
  return fit;
}
}  // namespace

LaneDetector::LaneDetector(const LaneDetectorParams & params)
: params_(params)
{
  computeTransforms();
}

void LaneDetector::computeTransforms()
{
  std::array<cv::Point2f, 4> dst{
    cv::Point2f{static_cast<float>(params_.warp_size.width) * 0.2f,
      static_cast<float>(params_.warp_size.height)},
    cv::Point2f{static_cast<float>(params_.warp_size.width) * 0.8f,
      static_cast<float>(params_.warp_size.height)},
    cv::Point2f{static_cast<float>(params_.warp_size.width) * 0.8f, 0.0f},
    cv::Point2f{static_cast<float>(params_.warp_size.width) * 0.2f, 0.0f}};

  perspective_M_ = cv::getPerspectiveTransform(params_.src_points.data(), dst.data());
  perspective_Minv_ = cv::getPerspectiveTransform(dst.data(), params_.src_points.data());
}

cv::Mat LaneDetector::threshold(const cv::Mat & bgr_frame) const
{
  // Classic combined gradient + color threshold (Sobel-x on grayscale for
  // edges, HLS S-channel for lane-marking color contrast that survives
  // shadows/track glare better than pure grayscale intensity).
  cv::Mat gray, hls, s_channel;
  cv::cvtColor(bgr_frame, gray, cv::COLOR_BGR2GRAY);
  cv::cvtColor(bgr_frame, hls, cv::COLOR_BGR2HLS);
  cv::extractChannel(hls, s_channel, 2);

  cv::Mat sobel_x, abs_sobel_x, sobel_8u;
  cv::Sobel(gray, sobel_x, CV_64F, 1, 0, 3);
  abs_sobel_x = cv::abs(sobel_x);
  double max_val;
  cv::minMaxLoc(abs_sobel_x, nullptr, &max_val);
  abs_sobel_x.convertTo(sobel_8u, CV_8U, max_val > 0 ? 255.0 / max_val : 0.0);

  cv::Mat sobel_mask, s_mask, combined;
  cv::inRange(sobel_8u, params_.sobel_thresh_min, params_.sobel_thresh_max, sobel_mask);
  cv::inRange(s_channel, params_.s_channel_thresh_min, params_.s_channel_thresh_max, s_mask);
  cv::bitwise_or(sobel_mask, s_mask, combined);
  return combined;
}

cv::Mat LaneDetector::warp(const cv::Mat & binary) const
{
#ifdef LANE_DETECTION_USE_CUDA
  cv::cuda::GpuMat gpu_in, gpu_out;
  gpu_in.upload(binary);
  cv::cuda::warpPerspective(gpu_in, gpu_out, perspective_M_, params_.warp_size);
  cv::Mat out;
  gpu_out.download(out);
  return out;
#else
  cv::Mat out;
  cv::warpPerspective(binary, out, perspective_M_, params_.warp_size);
  return out;
#endif
}

LaneFit LaneDetector::slidingWindowSearch(const cv::Mat & binary_warped, bool search_left_half) const
{
  const int height = binary_warped.rows;
  const int width = binary_warped.cols;

  // Histogram of the bottom half to find the starting x for this lane side.
  cv::Mat bottom_half = binary_warped(cv::Rect(0, height / 2, width, height / 2));
  cv::Mat col_sum;
  cv::reduce(bottom_half, col_sum, 0, cv::REDUCE_SUM, CV_32S);

  int search_start = search_left_half ? 0 : width / 2;
  int search_end = search_left_half ? width / 2 : width;

  int base_x = search_start;
  int best_val = -1;
  for (int x = search_start; x < search_end; ++x) {
    int v = col_sum.at<int>(0, x);
    if (v > best_val) {
      best_val = v;
      base_x = x;
    }
  }

  const int window_height = height / params_.n_windows;
  int current_x = base_x;
  std::vector<double> ys, xs;

  for (int w = 0; w < params_.n_windows; ++w) {
    int win_y_low = height - (w + 1) * window_height;
    int win_y_high = height - w * window_height;
    int win_x_low = std::max(0, current_x - params_.window_margin);
    int win_x_high = std::min(width, current_x + params_.window_margin);

    cv::Mat window = binary_warped(
      cv::Range(std::max(0, win_y_low), std::min(height, win_y_high)),
      cv::Range(win_x_low, win_x_high));

    std::vector<cv::Point> nonzero;
    cv::findNonZero(window, nonzero);

    if (!nonzero.empty()) {
      double sum_x = 0.0;
      for (const auto & p : nonzero) {
        ys.push_back(static_cast<double>(win_y_low + p.y));
        xs.push_back(static_cast<double>(win_x_low + p.x));
        sum_x += win_x_low + p.x;
      }
      if (static_cast<int>(nonzero.size()) > params_.min_pixels_to_recenter) {
        current_x = static_cast<int>(sum_x / static_cast<double>(nonzero.size()));
      }
    }
  }

  return fitPoly2(ys, xs);
}

double LaneDetector::estimateCurvatureRadius(const LaneFit & fit_px, double eval_y_px) const
{
  if (!fit_px.valid) {return 0.0;}
  // Re-fit in meter-space by scaling the pixel polynomial coefficients per
  // the standard Udacity/OpenCV lane-finding curvature formula:
  //   R = (1 + (2*a*y + b)^2)^1.5 / |2*a|
  // with a,b rescaled by the meters-per-pixel ratios.
  const double mx = params_.meters_per_pixel_x;
  const double my = params_.meters_per_pixel_y;
  double a_m = fit_px.a * (mx / (my * my));
  double b_m = fit_px.b * (mx / my);
  double y_m = eval_y_px * my;

  double denom = std::abs(2.0 * a_m);
  if (denom < 1e-9) {return 1e6;}  // effectively straight
  double numer = std::pow(1.0 + std::pow(2.0 * a_m * y_m + b_m, 2), 1.5);
  return numer / denom;
}

std::vector<cv::Point2d> LaneDetector::buildCenterline(const LaneFit & left, const LaneFit & right) const
{
  std::vector<cv::Point2d> centerline;
  if (!left.valid && !right.valid) {return centerline;}

  const int height = params_.warp_size.height;
  const int step = std::max(1, height / 40);

  for (int y_px = height - 1; y_px >= 0; y_px -= step) {
    double x_px;
    if (left.valid && right.valid) {
      x_px = 0.5 * (left.xAt(y_px) + right.xAt(y_px));
    } else if (left.valid) {
      // Only the left edge is visible: assume the known course lane_width_m
      // (NOT a generic road-lane guess) — this is the "road edge only"
      // fallback called out in the requirements.
      x_px = left.xAt(y_px) + (params_.lane_width_m / 2.0 / params_.meters_per_pixel_x);
    } else {
      x_px = right.xAt(y_px) - (params_.lane_width_m / 2.0 / params_.meters_per_pixel_x);
    }

    // Convert warped-image pixel (x_px, y_px) into vehicle-frame meters:
    // forward distance grows as y_px decreases (further up the image), and
    // is measured from the bottom (vehicle) row.
    double forward_m = static_cast<double>(height - y_px) * params_.meters_per_pixel_y;
    double lateral_px = x_px - static_cast<double>(params_.warp_size.width) / 2.0;
    double lateral_m = lateral_px * params_.meters_per_pixel_x;
    centerline.emplace_back(forward_m, lateral_m);
  }
  return centerline;
}

LaneResult LaneDetector::process(const cv::Mat & bgr_frame)
{
  LaneResult result;

  cv::Mat binary = threshold(bgr_frame);
  cv::Mat binary_warped = warp(binary);

  // Left/right lane searches are independent — run them concurrently to
  // use the Orin Nano's extra CPU cores instead of doubling per-frame
  // latency (Section J: multi-threading).
  auto left_future = std::async(std::launch::async,
    [this, &binary_warped]() {return slidingWindowSearch(binary_warped, true);});
  auto right_future = std::async(std::launch::async,
    [this, &binary_warped]() {return slidingWindowSearch(binary_warped, false);});

  result.left = left_future.get();
  result.right = right_future.get();

  double eval_y = static_cast<double>(params_.warp_size.height - 1);
  double r_left = estimateCurvatureRadius(result.left, eval_y);
  double r_right = estimateCurvatureRadius(result.right, eval_y);
  if (result.left.valid && result.right.valid) {
    result.curvature_radius_m = 0.5 * (r_left + r_right);
  } else {
    result.curvature_radius_m = result.left.valid ? r_left : r_right;
  }

  result.centerline_m = buildCenterline(result.left, result.right);

  if (result.left.valid && result.right.valid) {
    double lane_center_px =
      0.5 * (result.left.xAt(eval_y) + result.right.xAt(eval_y));
    double vehicle_center_px = static_cast<double>(params_.warp_size.width) / 2.0;
    result.center_offset_m = (vehicle_center_px - lane_center_px) * params_.meters_per_pixel_x;
  }

  if (params_.generate_debug_image) {
    cv::cvtColor(binary_warped, result.debug_image, cv::COLOR_GRAY2BGR);
    for (int y = 0; y < params_.warp_size.height; ++y) {
      if (result.left.valid) {
        int x = static_cast<int>(result.left.xAt(y));
        if (x >= 0 && x < result.debug_image.cols) {
          cv::circle(result.debug_image, {x, y}, 1, {0, 0, 255}, -1);
        }
      }
      if (result.right.valid) {
        int x = static_cast<int>(result.right.xAt(y));
        if (x >= 0 && x < result.debug_image.cols) {
          cv::circle(result.debug_image, {x, y}, 1, {255, 0, 0}, -1);
        }
      }
    }
  }

  return result;
}

}  // namespace lane_detection
