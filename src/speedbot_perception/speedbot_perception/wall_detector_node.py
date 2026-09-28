#!/usr/bin/env python3
"""
Wall Detection Node — SpeedBot Perception
==========================================
Detects track boundary walls (hay bales / barriers) from ZED 2i camera feed
using HSV color segmentation, morphological filtering, and Canny edges.

Outputs:
  - /perception/lane_center_offset  (Float64)  — lateral offset from track center in meters
  - /perception/wall_lines          (Marker)   — visualization of detected walls
  - /perception/debug_image         (Image)    — annotated debug image
  - /perception/track_boundaries    (Float64MultiArray) — left/right wall distances

Subscribes:
  - ZED RGB image
  - ZED Depth image
  - ZED CameraInfo
"""

import numpy as np
import cv2

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy

from sensor_msgs.msg import Image, CameraInfo
from std_msgs.msg import Float64, Float64MultiArray
from visualization_msgs.msg import Marker
from geometry_msgs.msg import Point
from cv_bridge import CvBridge


class WallDetectorNode(Node):
    """Detects track walls from camera imagery and publishes lane offset."""

    def __init__(self):
        super().__init__('wall_detector')

        # ── Declare Parameters ──
        self.declare_parameter('image_topic', '/zed/zed_node/rgb/color/rect/image')
        self.declare_parameter('depth_topic', '/zed/zed_node/depth/depth_registered')
        self.declare_parameter('camera_info_topic', '/zed/zed_node/rgb/color/rect/camera_info')

        self.declare_parameter('hsv_lower', [15, 40, 60])
        self.declare_parameter('hsv_upper', [45, 255, 255])

        self.declare_parameter('roi_top_fraction', 0.4)
        self.declare_parameter('roi_bottom_fraction', 1.0)

        self.declare_parameter('morph_kernel_size', 5)
        self.declare_parameter('morph_iterations', 2)

        self.declare_parameter('canny_low', 50)
        self.declare_parameter('canny_high', 150)

        self.declare_parameter('min_depth_m', 0.3)
        self.declare_parameter('max_depth_m', 8.0)

        self.declare_parameter('min_contour_area', 500)
        self.declare_parameter('polynomial_order', 2)
        self.declare_parameter('num_sample_points', 20)

        self.declare_parameter('publish_debug_image', True)
        self.declare_parameter('show_debug_window', False)  # false by default to prevent headless crash
        self.declare_parameter('record_video', False)        # record annotated detection to video file
        self.declare_parameter('video_output_path', '/home/Arnab/DIY_AntiGravity/speedbot_detection.mp4')
        self.declare_parameter('processing_rate_hz', 20.0)

        # ── Read Parameters ──
        self._load_params()

        # ── Bridge ──
        self.bridge = CvBridge()

        # ── Video Writer ──
        self.video_writer = None

        # ── Camera Intrinsics (populated from CameraInfo) ──
        self.fx = None
        self.fy = None
        self.cx = None
        self.cy = None

        # ── Latest frames ──
        self._latest_rgb = None
        self._latest_depth = None

        # ── QoS for ZED topics (best effort for camera streams) ──
        sensor_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=1
        )

        # ── Subscribers ──
        image_topic = self.get_parameter('image_topic').value
        depth_topic = self.get_parameter('depth_topic').value
        info_topic = self.get_parameter('camera_info_topic').value

        self.create_subscription(Image, image_topic, self._rgb_callback, sensor_qos)
        self.create_subscription(Image, depth_topic, self._depth_callback, sensor_qos)
        self.create_subscription(CameraInfo, info_topic, self._info_callback, 10)

        # ── Publishers ──
        self.pub_center_offset = self.create_publisher(Float64, '/perception/lane_center_offset', 10)
        self.pub_boundaries = self.create_publisher(Float64MultiArray, '/perception/track_boundaries', 10)
        self.pub_debug_img = self.create_publisher(Image, '/perception/debug_image', 1)
        self.pub_wall_markers = self.create_publisher(Marker, '/perception/wall_lines', 10)

        # ── Processing Timer ──
        rate = self.get_parameter('processing_rate_hz').value
        self.create_timer(1.0 / rate, self._process_frame)

        self.get_logger().info(
            f'WallDetector started | HSV [{self.hsv_lower}] → [{self.hsv_upper}] | '
            f'Rate: {rate} Hz'
        )

    def _load_params(self):
        """Load all parameters from the parameter server."""
        self.hsv_lower = np.array(self.get_parameter('hsv_lower').value, dtype=np.uint8)
        self.hsv_upper = np.array(self.get_parameter('hsv_upper').value, dtype=np.uint8)
        self.roi_top = self.get_parameter('roi_top_fraction').value
        self.roi_bottom = self.get_parameter('roi_bottom_fraction').value
        self.morph_k = self.get_parameter('morph_kernel_size').value
        self.morph_iter = self.get_parameter('morph_iterations').value
        self.canny_low = self.get_parameter('canny_low').value
        self.canny_high = self.get_parameter('canny_high').value
        self.min_depth = self.get_parameter('min_depth_m').value
        self.max_depth = self.get_parameter('max_depth_m').value
        self.min_contour_area = self.get_parameter('min_contour_area').value
        self.poly_order = self.get_parameter('polynomial_order').value
        self.n_sample_pts = self.get_parameter('num_sample_points').value
        self.publish_debug = self.get_parameter('publish_debug_image').value
        self.show_window = self.get_parameter('show_debug_window').value
        self.record_video = self.get_parameter('record_video').value
        self.video_output_path = self.get_parameter('video_output_path').value
        self.rate_hz = self.get_parameter('processing_rate_hz').value

    # ──────────────────────────────────────────────
    #  Callbacks
    # ──────────────────────────────────────────────
    def _rgb_callback(self, msg: Image):
        self._latest_rgb = msg

    def _depth_callback(self, msg: Image):
        self._latest_depth = msg

    def _info_callback(self, msg: CameraInfo):
        if self.fx is None:
            self.fx = msg.k[0]
            self.fy = msg.k[4]
            self.cx = msg.k[2]
            self.cy = msg.k[5]
            self.get_logger().info(
                f'Camera intrinsics: fx={self.fx:.1f} fy={self.fy:.1f} '
                f'cx={self.cx:.1f} cy={self.cy:.1f}'
            )

    # ──────────────────────────────────────────────
    #  Main Processing Pipeline
    # ──────────────────────────────────────────────
    def _process_frame(self):
        """Run the full perception pipeline on the latest frames."""
        if self._latest_rgb is None:
            return

        # Convert ROS Image → OpenCV
        try:
            rgb_frame = self.bridge.imgmsg_to_cv2(self._latest_rgb, 'bgr8')
        except Exception as e:
            self.get_logger().warn(f'Failed to convert RGB: {e}')
            return

        depth_frame = None
        if self._latest_depth is not None:
            try:
                depth_frame = self.bridge.imgmsg_to_cv2(self._latest_depth, '32FC1')
            except Exception as e:
                self.get_logger().warn(f'Failed to convert depth: {e}')

        h, w = rgb_frame.shape[:2]

        # ── Step 1: Region of Interest ──
        roi_y_start = int(h * self.roi_top)
        roi_y_end = int(h * self.roi_bottom)
        roi = rgb_frame[roi_y_start:roi_y_end, :]

        # ── Step 2: HSV Color Segmentation ──
        hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
        mask = cv2.inRange(hsv, self.hsv_lower, self.hsv_upper)

        # ── Step 3: Morphological Cleanup ──
        kernel = cv2.getStructuringElement(
            cv2.MORPH_RECT, (self.morph_k, self.morph_k)
        )
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=self.morph_iter)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=self.morph_iter)

        # ── Step 4: Canny Edge Detection on Mask ──
        edges = cv2.Canny(mask, self.canny_low, self.canny_high)

        # ── Step 5: Find Contours → Separate Left/Right Walls ──
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        contours = [c for c in contours if cv2.contourArea(c) > self.min_contour_area]

        roi_w = roi.shape[1]
        roi_h = roi.shape[0]
        mid_x = roi_w // 2

        left_points = []
        right_points = []

        for contour in contours:
            for point in contour:
                px, py = point[0]
                if px < mid_x:
                    # Left wall: take the rightmost (inner) edge
                    left_points.append((px, py))
                else:
                    # Right wall: take the leftmost (inner) edge
                    right_points.append((px, py))

        # ── Step 6: Fit Polynomial to Each Wall ──
        left_wall_x = None
        right_wall_x = None
        left_fit = None
        right_fit = None

        y_samples = np.linspace(0, roi_h - 1, self.n_sample_pts).astype(int)

        if len(left_points) > self.poly_order + 1:
            lp = np.array(left_points)
            # For each y-row, take the maximum x (innermost edge of left wall)
            left_fit = np.polyfit(lp[:, 1], lp[:, 0], self.poly_order)
            left_wall_x = np.polyval(left_fit, y_samples)

        if len(right_points) > self.poly_order + 1:
            rp = np.array(right_points)
            # For each y-row, take the minimum x (innermost edge of right wall)
            right_fit = np.polyfit(rp[:, 1], rp[:, 0], self.poly_order)
            right_wall_x = np.polyval(right_fit, y_samples)

        # ── Step 7: Compute Lane Center Offset ──
        offset_msg = Float64()
        boundary_msg = Float64MultiArray()

        if left_wall_x is not None and right_wall_x is not None:
            # Track center in pixel space
            center_x = (left_wall_x + right_wall_x) / 2.0
            # Average offset from image center (normalized to [-1, 1])
            avg_center = np.mean(center_x)
            pixel_offset = avg_center - mid_x
            normalized_offset = pixel_offset / mid_x  # -1 = far left, +1 = far right

            offset_msg.data = float(normalized_offset)

            # Also compute approximate left/right distances in meters using depth
            left_dist_m, right_dist_m = self._estimate_wall_distances(
                depth_frame, left_wall_x, right_wall_x, y_samples,
                roi_y_start
            )
            boundary_msg.data = [left_dist_m, right_dist_m]

        elif left_wall_x is not None:
            # Only left wall visible — steer right
            avg_left = np.mean(left_wall_x)
            offset_msg.data = -0.5  # bias right
            boundary_msg.data = [float(avg_left / roi_w), -1.0]

        elif right_wall_x is not None:
            # Only right wall visible — steer left
            avg_right = np.mean(right_wall_x)
            offset_msg.data = 0.5  # bias left
            boundary_msg.data = [-1.0, float(avg_right / roi_w)]

        else:
            # No walls detected — maintain course
            offset_msg.data = 0.0
            boundary_msg.data = [-1.0, -1.0]

        self.pub_center_offset.publish(offset_msg)
        self.pub_boundaries.publish(boundary_msg)

        # ── Step 8: Debug Visualization ──
        if self.publish_debug:
            self._publish_debug(
                rgb_frame, roi_y_start, roi_y_end, mask, contours,
                left_wall_x, right_wall_x, y_samples, offset_msg.data
            )

    # ──────────────────────────────────────────────
    #  Depth-Based Wall Distance Estimation
    # ──────────────────────────────────────────────
    def _estimate_wall_distances(self, depth_frame, left_x, right_x,
                                  y_samples, roi_y_offset):
        """Estimate left and right wall distances in meters using the depth map."""
        left_dist = -1.0
        right_dist = -1.0

        if depth_frame is None or self.fx is None:
            return left_dist, right_dist

        h, w = depth_frame.shape[:2]

        # Sample depth at wall pixel locations
        left_depths = []
        right_depths = []

        for i, y in enumerate(y_samples):
            img_y = int(y + roi_y_offset)
            if img_y < 0 or img_y >= h:
                continue

            # Left wall
            if left_x is not None:
                lx = int(np.clip(left_x[i], 0, w - 1))
                d = depth_frame[img_y, lx]
                if self.min_depth < d < self.max_depth:
                    # Convert pixel offset to meters: x_meters = (px - cx) * depth / fx
                    x_m = (lx - self.cx) * d / self.fx
                    left_depths.append(abs(x_m))

            # Right wall
            if right_x is not None:
                rx = int(np.clip(right_x[i], 0, w - 1))
                d = depth_frame[img_y, rx]
                if self.min_depth < d < self.max_depth:
                    x_m = (rx - self.cx) * d / self.fx
                    right_depths.append(abs(x_m))

        if left_depths:
            left_dist = float(np.median(left_depths))
        if right_depths:
            right_dist = float(np.median(right_depths))

        return left_dist, right_dist

    # ──────────────────────────────────────────────
    #  Debug Image Publisher
    # ──────────────────────────────────────────────
    def _publish_debug(self, frame, roi_top, roi_bot, mask, contours,
                       left_x, right_x, y_samples, offset):
        """Draw annotations on the frame and publish as debug image."""
        debug = frame.copy()

        # Draw ROI bounds
        cv2.line(debug, (0, roi_top), (debug.shape[1], roi_top), (255, 255, 0), 1)

        # Draw contours in ROI region
        for c in contours:
            shifted = c.copy()
            shifted[:, :, 1] += roi_top
            cv2.drawContours(debug, [shifted], -1, (0, 255, 0), 2)

        # Draw fitted wall lines
        if left_x is not None:
            for i in range(len(y_samples) - 1):
                pt1 = (int(left_x[i]), int(y_samples[i] + roi_top))
                pt2 = (int(left_x[i + 1]), int(y_samples[i + 1] + roi_top))
                cv2.line(debug, pt1, pt2, (255, 0, 0), 3)  # Blue = left wall

        if right_x is not None:
            for i in range(len(y_samples) - 1):
                pt1 = (int(right_x[i]), int(y_samples[i] + roi_top))
                pt2 = (int(right_x[i + 1]), int(y_samples[i + 1] + roi_top))
                cv2.line(debug, pt1, pt2, (0, 0, 255), 3)  # Red = right wall

        # Draw track center line
        if left_x is not None and right_x is not None:
            center_x = (left_x + right_x) / 2.0
            for i in range(len(y_samples) - 1):
                pt1 = (int(center_x[i]), int(y_samples[i] + roi_top))
                pt2 = (int(center_x[i + 1]), int(y_samples[i + 1] + roi_top))
                cv2.line(debug, pt1, pt2, (0, 255, 255), 2)  # Yellow = center

        # Draw offset text
        direction = "LEFT" if offset > 0 else "RIGHT" if offset < 0 else "CENTER"
        cv2.putText(
            debug, f'Offset: {offset:.3f} ({direction})',
            (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 255), 2
        )

        # Publish to ROS topic
        try:
            debug_msg = self.bridge.cv2_to_imgmsg(debug, 'bgr8')
            debug_msg.header.stamp = self.get_clock().now().to_msg()
            self.pub_debug_img.publish(debug_msg)
        except Exception as e:
            self.get_logger().warn(f'Failed to publish debug image: {e}')

        # Record annotated detection to video file
        if self.record_video:
            try:
                if self.video_writer is None:
                    h, w = debug.shape[:2]
                    fourcc = cv2.VideoWriter_fourcc(*'mp4v')
                    self.video_writer = cv2.VideoWriter(
                        self.video_output_path, fourcc, float(self.rate_hz), (w, h)
                    )
                    self.get_logger().info(f'🎥 Started recording detection video: {self.video_output_path}')
                self.video_writer.write(debug)
            except Exception as e:
                self.get_logger().warn(f'Failed to write video frame: {e}')

        # Show in OpenCV window (for on-robot debugging)
        if self.show_window:
            try:
                # Create side-by-side: debug image | HSV mask (colorized)
                mask_color = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
                # Resize mask to match debug frame height
                mask_resized = cv2.resize(mask_color, (debug.shape[1] // 3, debug.shape[0]))
                debug_resized = cv2.resize(debug, (debug.shape[1] * 2 // 3, debug.shape[0]))
                combined = np.hstack([debug_resized, mask_resized])

                cv2.imshow('SpeedBot Wall Detector', combined)
                key = cv2.waitKey(1) & 0xFF
                if key == ord('q'):
                    self.get_logger().info('Quit key pressed — shutting down')
                    raise SystemExit
            except cv2.error as e:
                self.get_logger().warn(f'Could not open GUI window (headless/no DISPLAY): {e}')
                self.show_window = False
            except Exception as e:
                if isinstance(e, SystemExit):
                    raise
                self.get_logger().warn(f'Display window error: {e}')
                self.show_window = False

    def destroy_node(self):
        """Release video writer and clean up resources."""
        if self.video_writer is not None:
            self.video_writer.release()
            self.get_logger().info(f'✅ Video saved successfully: {self.video_output_path}')
        super().destroy_node()


def main(args=None):
    rclpy.init(args=args)
    node = WallDetectorNode()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, SystemExit):
        pass
    finally:
        cv2.destroyAllWindows()
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
