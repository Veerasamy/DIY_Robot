#!/usr/bin/env python3
"""
LiDAR Processor Node — SpeedBot Perception
============================================
Processes RPLiDAR C1 2D scan data for:
  1. Range filtering (ignore out-of-range points)
  2. Obstacle clustering (DBSCAN)
  3. Follow-the-Gap algorithm — finds the widest open gap for steering
  4. Safety bubble around closest obstacle

Outputs:
  - /perception/best_gap_angle     (Float64)  — best steering angle in radians
  - /perception/closest_obstacle    (Float64)  — distance to nearest obstacle in meters
  - /perception/obstacle_markers    (MarkerArray) — RViz visualization of clusters
  - /perception/filtered_scan       (LaserScan)   — filtered scan for downstream use

Subscribes:
  - /scan (RPLiDAR C1 LaserScan)
"""

import numpy as np
import math

import rclpy
from rclpy.node import Node
from rclpy.qos import QoSProfile, ReliabilityPolicy, HistoryPolicy

from sensor_msgs.msg import LaserScan
from std_msgs.msg import Float64, ColorRGBA
from visualization_msgs.msg import Marker, MarkerArray
from geometry_msgs.msg import Point
from builtin_interfaces.msg import Duration


class LidarProcessorNode(Node):
    """Processes 2D LiDAR scans for obstacle detection and gap finding."""

    def __init__(self):
        super().__init__('lidar_processor')

        # ── Declare Parameters ──
        self.declare_parameter('scan_topic', '/scan')

        self.declare_parameter('range_min', 0.15)
        self.declare_parameter('range_max', 12.0)
        self.declare_parameter('angle_min_deg', -135.0)
        self.declare_parameter('angle_max_deg', 135.0)

        self.declare_parameter('cluster_eps', 0.15)
        self.declare_parameter('cluster_min_samples', 3)

        self.declare_parameter('front_collision_angle_deg', 35.0)
        self.declare_parameter('safety_bubble_radius', 0.15)
        self.declare_parameter('max_gap_angle_deg', 90.0)

        self.declare_parameter('publish_debug_markers', True)
        self.declare_parameter('processing_rate_hz', 20.0)

        # ── Load Parameters ──
        self.range_min = self.get_parameter('range_min').value
        self.range_max = self.get_parameter('range_max').value
        self.angle_min = math.radians(self.get_parameter('angle_min_deg').value)
        self.angle_max = math.radians(self.get_parameter('angle_max_deg').value)
        self.front_collision_angle = math.radians(self.get_parameter('front_collision_angle_deg').value)
        self.cluster_eps = self.get_parameter('cluster_eps').value
        self.cluster_min_samples = self.get_parameter('cluster_min_samples').value
        self.safety_bubble = self.get_parameter('safety_bubble_radius').value
        self.max_gap_angle = math.radians(self.get_parameter('max_gap_angle_deg').value)
        self.publish_markers = self.get_parameter('publish_debug_markers').value

        self._latest_scan = None

        # ── Subscribers ──
        sensor_qos = QoSProfile(
            reliability=ReliabilityPolicy.BEST_EFFORT,
            history=HistoryPolicy.KEEP_LAST,
            depth=1
        )
        scan_topic = self.get_parameter('scan_topic').value
        self.create_subscription(LaserScan, scan_topic, self._scan_callback, sensor_qos)

        # ── Publishers ──
        self.pub_gap_angle = self.create_publisher(Float64, '/perception/best_gap_angle', 10)
        self.pub_closest = self.create_publisher(Float64, '/perception/closest_obstacle', 10)
        self.pub_filtered_scan = self.create_publisher(LaserScan, '/perception/filtered_scan', 10)
        self.pub_markers = self.create_publisher(MarkerArray, '/perception/obstacle_markers', 10)

        # ── Processing Timer ──
        rate = self.get_parameter('processing_rate_hz').value
        self.create_timer(1.0 / rate, self._process_scan)

        self.get_logger().info(
            f'LidarProcessor started | Range: [{self.range_min:.2f}, {self.range_max:.2f}]m | '
            f'Arc: [{math.degrees(self.angle_min):.0f}°, {math.degrees(self.angle_max):.0f}°] | '
            f'Front Arc: ±{math.degrees(self.front_collision_angle):.0f}° | '
            f'Rate: {rate} Hz'
        )

    def _scan_callback(self, msg: LaserScan):
        self._latest_scan = msg

    def _process_scan(self):
        """Run the full LiDAR processing pipeline."""
        if self._latest_scan is None:
            return

        scan = self._latest_scan
        ranges = np.array(scan.ranges, dtype=np.float32)
        n_points = len(ranges)

        # Compute angle for each range reading
        angles = np.array([
            scan.angle_min + i * scan.angle_increment
            for i in range(n_points)
        ], dtype=np.float32)

        # ── Step 1: Filter by Range and Angle ──
        valid = (
            (ranges >= self.range_min) &
            (ranges <= self.range_max) &
            (angles >= self.angle_min) &
            (angles <= self.angle_max) &
            np.isfinite(ranges)
        )

        filtered_ranges = ranges.copy()
        filtered_ranges[~valid] = 0.0  # mark invalid as 0

        # Publish filtered scan
        filtered_scan = LaserScan()
        filtered_scan.header = scan.header
        filtered_scan.angle_min = scan.angle_min
        filtered_scan.angle_max = scan.angle_max
        filtered_scan.angle_increment = scan.angle_increment
        filtered_scan.time_increment = scan.time_increment
        filtered_scan.scan_time = scan.scan_time
        filtered_scan.range_min = self.range_min
        filtered_scan.range_max = self.range_max
        filtered_scan.ranges = filtered_ranges.tolist()
        self.pub_filtered_scan.publish(filtered_scan)

        # ── Step 2: Find Closest Frontal Obstacle (for collision braking) ──
        front_mask = valid & (np.abs(angles) <= self.front_collision_angle)
        front_ranges = filtered_ranges[front_mask]

        closest_msg = Float64()
        if len(front_ranges) > 0:
            closest_msg.data = float(np.min(front_ranges))
        else:
            closest_msg.data = float(self.range_max)
        self.pub_closest.publish(closest_msg)

        valid_ranges = filtered_ranges[valid]
        valid_angles = angles[valid]

        # ── Step 3: Safety Bubble ──
        # Zero out all points within safety_bubble radius of the closest point
        bubble_ranges = filtered_ranges.copy()
        if len(valid_ranges) > 0:
            closest_idx = np.argmin(filtered_ranges[valid])
            # Map back to original index space
            valid_indices = np.where(valid)[0]
            closest_global_idx = valid_indices[closest_idx]

            # Apply safety bubble: zero out nearby points
            closest_angle = angles[closest_global_idx]
            closest_range = filtered_ranges[closest_global_idx]

            for i in range(n_points):
                if bubble_ranges[i] == 0:
                    continue
                # Euclidean distance between this point and closest point
                dx = bubble_ranges[i] * math.cos(angles[i]) - closest_range * math.cos(closest_angle)
                dy = bubble_ranges[i] * math.sin(angles[i]) - closest_range * math.sin(closest_angle)
                dist = math.sqrt(dx * dx + dy * dy)
                if dist < self.safety_bubble:
                    bubble_ranges[i] = 0.0

        # ── Step 4: Follow-the-Gap ──
        gap_angle = self._follow_the_gap(bubble_ranges, angles)
        gap_msg = Float64()
        gap_msg.data = float(gap_angle)
        self.pub_gap_angle.publish(gap_msg)

        # ── Step 5: Obstacle Clustering (DBSCAN-like) ──
        if self.publish_markers and len(valid_ranges) > 0:
            clusters = self._cluster_obstacles(valid_ranges, valid_angles)
            self._publish_obstacle_markers(clusters, scan.header)

    def _follow_the_gap(self, ranges, angles):
        """
        Follow-the-Gap algorithm.
        Finds the widest contiguous gap of non-zero ranges and returns
        the angle pointing to the center of that gap.
        """
        n = len(ranges)
        if n == 0:
            return 0.0

        # Find contiguous runs of non-zero ranges
        best_gap_start = 0
        best_gap_length = 0
        current_start = -1
        current_length = 0

        for i in range(n):
            if ranges[i] > 0 and abs(angles[i]) <= self.max_gap_angle:
                if current_start < 0:
                    current_start = i
                    current_length = 1
                else:
                    current_length += 1
            else:
                if current_length > best_gap_length:
                    best_gap_start = current_start
                    best_gap_length = current_length
                current_start = -1
                current_length = 0

        # Check last run
        if current_length > best_gap_length:
            best_gap_start = current_start
            best_gap_length = current_length

        if best_gap_length == 0:
            return 0.0

        # Find the deepest point within the best gap (most distant = safest)
        gap_end = best_gap_start + best_gap_length
        gap_ranges = ranges[best_gap_start:gap_end]
        gap_angles = angles[best_gap_start:gap_end]

        # Weighted center: bias toward deeper (farther) points
        if np.sum(gap_ranges) > 0:
            weighted_angle = np.average(gap_angles, weights=gap_ranges)
        else:
            weighted_angle = np.mean(gap_angles)

        return weighted_angle

    def _cluster_obstacles(self, ranges, angles):
        """
        Simple single-linkage clustering on 2D LiDAR points.
        Groups nearby points into obstacle clusters.
        Returns list of clusters, each a list of (x, y) tuples.
        """
        # Convert polar → Cartesian
        xs = ranges * np.cos(angles)
        ys = ranges * np.sin(angles)
        points = list(zip(xs, ys))

        # Single-linkage clustering
        clusters = []
        visited = [False] * len(points)

        for i in range(len(points)):
            if visited[i]:
                continue

            cluster = [points[i]]
            visited[i] = True
            queue = [i]

            while queue:
                current = queue.pop(0)
                cx, cy = points[current]

                for j in range(len(points)):
                    if visited[j]:
                        continue
                    jx, jy = points[j]
                    dist = math.sqrt((cx - jx) ** 2 + (cy - jy) ** 2)
                    if dist <= self.cluster_eps:
                        visited[j] = True
                        cluster.append(points[j])
                        queue.append(j)

            if len(cluster) >= self.cluster_min_samples:
                clusters.append(cluster)

        return clusters

    def _publish_obstacle_markers(self, clusters, header):
        """Publish obstacle clusters as RViz MarkerArray."""
        marker_array = MarkerArray()

        # First, clear old markers
        clear_marker = Marker()
        clear_marker.header = header
        clear_marker.action = Marker.DELETEALL
        marker_array.markers.append(clear_marker)

        colors = [
            ColorRGBA(r=1.0, g=0.0, b=0.0, a=0.8),
            ColorRGBA(r=0.0, g=1.0, b=0.0, a=0.8),
            ColorRGBA(r=0.0, g=0.0, b=1.0, a=0.8),
            ColorRGBA(r=1.0, g=1.0, b=0.0, a=0.8),
            ColorRGBA(r=1.0, g=0.0, b=1.0, a=0.8),
        ]

        for idx, cluster in enumerate(clusters):
            marker = Marker()
            marker.header = header
            marker.ns = 'obstacles'
            marker.id = idx + 1
            marker.type = Marker.POINTS
            marker.action = Marker.ADD
            marker.scale.x = 0.08
            marker.scale.y = 0.08
            marker.color = colors[idx % len(colors)]
            marker.lifetime = Duration(sec=0, nanosec=200_000_000)  # 200ms

            for (x, y) in cluster:
                p = Point()
                p.x = float(x)
                p.y = float(y)
                p.z = 0.0
                marker.points.append(p)

            marker_array.markers.append(marker)

        self.pub_markers.publish(marker_array)


def main(args=None):
    rclpy.init(args=args)
    node = LidarProcessorNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        pass
    finally:
        node.destroy_node()
        if rclpy.ok():
            rclpy.shutdown()


if __name__ == '__main__':
    main()
