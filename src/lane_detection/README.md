# lane_detection

CUDA-accelerated OpenCV lane / road-edge detection producing a centerline
path and curvature estimate for the `controller` package.

## Pipeline

1. **Threshold** — combined Sobel-X gradient (edges) + HLS S-channel
   (color contrast robust to shadows/glare) binary mask.
2. **Perspective warp** — maps the four calibrated source points in the raw
   camera image to a bird's-eye rectangle (`LaneDetectorParams::src_points`).
   Recalibrate these per camera mounting angle/height (see below).
3. **Sliding-window search** — histogram-seeded window search per lane side,
   run concurrently via `std::async` (left/right are independent —
   Section J multi-threading).
4. **2nd-order polynomial fit** (`x = a*y^2 + b*y + c`) per side via
   least-squares (`cv::solve`, SVD).
5. **Curvature** — standard closed-form radius from the polynomial
   coefficients, rescaled into meters using `meters_per_pixel_x/y`.
6. **Centerline** — midpoint of both fits when both lane edges are visible;
   falls back to an offset from whichever single edge is visible
   ("road edge detection" requirement) using a nominal 1.85m half lane width.

## CUDA acceleration

Built with `LANE_DETECTION_USE_CUDA` when OpenCV's `cudaarithm` /
`cudaimgproc` / `cudawarping` modules are found (JetPack's OpenCV build
usually includes these). The perspective warp runs on GPU; the
threshold/sliding-window stages remain CPU because they are already
sub-millisecond and porting them to GPU would add more upload/download
overhead than they'd save at this resolution.

## Calibrating `src_points`

1. Launch with `publish_debug_image:=true` and view `/lane/debug_image` +
   the raw `/zed/rgb/image_raw` in rviz side by side.
2. Pick four points in the raw image that form a rectangle on the physical
   track (e.g. lane-marking corners a known distance apart).
3. Update `src_points` and `meters_per_pixel_x/y` (measure the real-world
   width/length of that rectangle) via the node's parameters.

## Topics

- Subscribes: `/zed/rgb/image_raw`
- Publishes: `/lane/centerline` (`nav_msgs/Path`, vehicle frame, meters),
  `/lane/curvature_radius_m` (`std_msgs/Float32`),
  `/lane/center_offset_m` (`std_msgs/Float32`),
  `/lane/debug_image` (optional)
