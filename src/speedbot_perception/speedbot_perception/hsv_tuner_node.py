#!/usr/bin/env python3
"""
HSV Tuner Node — SpeedBot Perception
=====================================
Interactive GUI with trackbars to tune HSV color filter thresholds in real time.
This helps you dial in the exact color range for your track boundaries on-site.

Usage:
    ros2 run speedbot_perception hsv_tuner

Keys:
    's' — Save current HSV values to console (copy to perception_params.yaml)
    'q' — Quit

Subscribes:
    - ZED RGB image
"""

import numpy as np
import cv2

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy

from sensor_msgs.msg import Image
from cv_bridge import CvBridge


class HSVTunerNode(Node):
    """Interactive HSV threshold tuner supporting both OpenCV GUI and headless ROS parameters."""

    def __init__(self):
        super().__init__('hsv_tuner')

        self.declare_parameter('image_topic', '/zed/zed_node/rgb/color/rect/image')
        self.declare_parameter('roi_top_fraction', 0.4)
        self.declare_parameter('roi_bottom_fraction', 1.0)
        self.declare_parameter('h_low', 15)
        self.declare_parameter('s_low', 40)
        self.declare_parameter('v_low', 60)
        self.declare_parameter('h_high', 45)
        self.declare_parameter('s_high', 255)
        self.declare_parameter('v_high', 255)

        self.bridge = CvBridge()
        self._latest_frame = None

        # Default HSV range
        self.h_low = self.get_parameter('h_low').value
        self.s_low = self.get_parameter('s_low').value
        self.v_low = self.get_parameter('v_low').value
        self.h_high = self.get_parameter('h_high').value
        self.s_high = self.get_parameter('s_high').value
        self.v_high = self.get_parameter('v_high').value

        self.roi_top = self.get_parameter('roi_top_fraction').value
        self.roi_bottom = self.get_parameter('roi_bottom_fraction').value

        # ── Publishers ──
        self.pub_preview = self.create_publisher(Image, '/perception/hsv_preview', 1)

        # ── Try Create GUI Window ──
        self.gui_available = False
        try:
            cv2.namedWindow('HSV Tuner', cv2.WINDOW_NORMAL)
            cv2.resizeWindow('HSV Tuner', 800, 600)

            cv2.createTrackbar('H Low', 'HSV Tuner', self.h_low, 179, self._on_trackbar)
            cv2.createTrackbar('S Low', 'HSV Tuner', self.s_low, 255, self._on_trackbar)
            cv2.createTrackbar('V Low', 'HSV Tuner', self.v_low, 255, self._on_trackbar)
            cv2.createTrackbar('H High', 'HSV Tuner', self.h_high, 179, self._on_trackbar)
            cv2.createTrackbar('S High', 'HSV Tuner', self.s_high, 255, self._on_trackbar)
            cv2.createTrackbar('V High', 'HSV Tuner', self.v_high, 255, self._on_trackbar)
            self.gui_available = True
        except cv2.error as e:
            self.get_logger().warn(
                f'No X11 display found ({e}). Running in HEADLESS mode.\n'
                f'  Preview topic: /perception/hsv_preview\n'
                f'  Tune live using: ros2 param set /hsv_tuner h_low <val>'
            )

        # ── Subscribe ──
        sensor_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=1
        )
        image_topic = self.get_parameter('image_topic').value
        self.create_subscription(Image, image_topic, self._image_callback, sensor_qos)

        # ── GUI / Preview update timer (20 Hz) ──
        self.create_timer(1.0 / 20.0, self._update_gui)

        self.get_logger().info(
            f'HSV Tuner running. Subscribed to: {image_topic}\n'
            f'  Publishing preview to: /perception/hsv_preview'
        )

    def _on_trackbar(self, _):
        """Dummy callback — values are read in _update_gui."""
        pass

    def _image_callback(self, msg: Image):
        try:
            self._latest_frame = self.bridge.imgmsg_to_cv2(msg, 'bgr8')
        except Exception as e:
            self.get_logger().warn(f'Image conversion failed: {e}')

    def _update_gui(self):
        """Read values, apply HSV mask, publish preview and display window."""
        if self._latest_frame is None:
            return

        frame = self._latest_frame.copy()
        h, w = frame.shape[:2]

        # Apply ROI
        roi_y_start = int(h * self.roi_top)
        roi_y_end = int(h * self.roi_bottom)
        roi = frame[roi_y_start:roi_y_end, :]

        # Read values from GUI trackbars or ROS parameters
        if self.gui_available:
            try:
                self.h_low = cv2.getTrackbarPos('H Low', 'HSV Tuner')
                self.s_low = cv2.getTrackbarPos('S Low', 'HSV Tuner')
                self.v_low = cv2.getTrackbarPos('V Low', 'HSV Tuner')
                self.h_high = cv2.getTrackbarPos('H High', 'HSV Tuner')
                self.s_high = cv2.getTrackbarPos('S High', 'HSV Tuner')
                self.v_high = cv2.getTrackbarPos('V High', 'HSV Tuner')
            except cv2.error:
                self.gui_available = False
        else:
            self.h_low = self.get_parameter('h_low').value
            self.s_low = self.get_parameter('s_low').value
            self.v_low = self.get_parameter('v_low').value
            self.h_high = self.get_parameter('h_high').value
            self.s_high = self.get_parameter('s_high').value
            self.v_high = self.get_parameter('v_high').value

        lower = np.array([self.h_low, self.s_low, self.v_low], dtype=np.uint8)
        upper = np.array([self.h_high, self.s_high, self.v_high], dtype=np.uint8)

        # Apply HSV filter
        hsv = cv2.cvtColor(roi, cv2.COLOR_BGR2HSV)
        mask = cv2.inRange(hsv, lower, upper)

        # Morphological cleanup
        kernel = cv2.getStructuringElement(cv2.MORPH_RECT, (5, 5))
        mask = cv2.morphologyEx(mask, cv2.MORPH_CLOSE, kernel, iterations=2)
        mask = cv2.morphologyEx(mask, cv2.MORPH_OPEN, kernel, iterations=2)

        # Create colored overlay
        colored_mask = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
        overlay = cv2.addWeighted(roi, 0.6, colored_mask, 0.4, 0)

        # Stack: original ROI | mask | overlay
        mask_3ch = cv2.cvtColor(mask, cv2.COLOR_GRAY2BGR)
        display = np.hstack([roi, mask_3ch, overlay])

        # Add text
        text = f'HSV Lower: [{self.h_low}, {self.s_low}, {self.v_low}]  Upper: [{self.h_high}, {self.s_high}, {self.v_high}]'
        cv2.putText(display, text, (10, 25), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 255, 255), 2)

        # Publish preview image over ROS
        try:
            prev_msg = self.bridge.cv2_to_imgmsg(display, 'bgr8')
            prev_msg.header.stamp = self.get_clock().now().to_msg()
            self.pub_preview.publish(prev_msg)
        except Exception:
            pass

        # If local GUI is open, display window
        if self.gui_available:
            try:
                cv2.imshow('HSV Tuner', display)
                key = cv2.waitKey(1) & 0xFF
                if key == ord('s'):
                    self._save_values()
                elif key == ord('q'):
                    self.get_logger().info('Quitting HSV Tuner...')
                    cv2.destroyAllWindows()
                    raise SystemExit
            except cv2.error:
                self.gui_available = False

    def _save_values(self):
        """Print current HSV values in YAML format for easy copy-paste."""
        self.get_logger().info(
            '\n╔═══════════════════════════════════════════╗\n'
            '║        SAVED HSV VALUES                   ║\n'
            '╠═══════════════════════════════════════════╣\n'
            f'║  hsv_lower: [{self.h_low}, {self.s_low}, {self.v_low}]'
            f'{" " * (24 - len(f"[{self.h_low}, {self.s_low}, {self.v_low}]"))}║\n'
            f'║  hsv_upper: [{self.h_high}, {self.s_high}, {self.v_high}]'
            f'{" " * (24 - len(f"[{self.h_high}, {self.s_high}, {self.v_high}]"))}║\n'
            '╚═══════════════════════════════════════════╝\n'
            'Copy the above values into perception_params.yaml'
        )


def main(args=None):
    rclpy.init(args=args)
    node = HSVTunerNode()
    try:
        rclpy.spin(node)
    except (KeyboardInterrupt, SystemExit):
        pass
    finally:
        cv2.destroyAllWindows()
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
