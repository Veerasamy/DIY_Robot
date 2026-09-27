#!/usr/bin/env python3
"""Smart build wrapper for the autonomous_rc colcon workspace.

Validates the toolchain (ROS2, CUDA, ZED SDK, OpenCV, Eigen3) before invoking
colcon so build failures surface as clear, actionable messages instead of
cryptic CMake errors. Intended to be the single entry point for CI and local
development on the Jetson Orin Nano / Ubuntu 24.04 target.
"""
import argparse
import os
import shutil
import subprocess
import sys
from pathlib import Path

WORKSPACE_ROOT = Path(__file__).resolve().parent
SRC_DIR = WORKSPACE_ROOT / "src"


def _which(binary: str) -> str | None:
    return shutil.which(binary)


def check_environment(verbose: bool = True) -> bool:
    """Validate the presence of required toolchain components.

    Returns True if the environment is sufficient to build all packages,
    False otherwise (build.py still allows a --force build for CI images
    that install dependencies in a later stage).
    """
    checks: list[tuple[str, bool, str]] = []

    ros_distro = os.environ.get("ROS_DISTRO")
    checks.append(("ROS2 sourced (ROS_DISTRO env)", ros_distro is not None,
                    "run: source /opt/ros/jazzy/setup.bash"))

    checks.append(("colcon", _which("colcon") is not None,
                    "run: sudo apt install python3-colcon-common-extensions"))

    checks.append(("cmake", _which("cmake") is not None,
                    "run: sudo apt install cmake"))

    nvcc = _which("nvcc")
    checks.append(("CUDA toolchain (nvcc)", nvcc is not None,
                    "install CUDA 12.x (JetPack ships this on Orin Nano)"))

    zed_sdk = Path("/usr/local/zed/include/sl/Camera.hpp")
    checks.append(("ZED SDK headers", zed_sdk.exists(),
                    "install the ZED SDK from stereolabs.com matching your CUDA/L4T version"))

    opencv_pc = shutil.which("pkg-config") and subprocess.run(
        ["pkg-config", "--exists", "opencv4"], capture_output=True
    ).returncode == 0
    checks.append(("OpenCV 4 (pkg-config opencv4)", bool(opencv_pc),
                    "run: sudo apt install libopencv-dev"))

    eigen = Path("/usr/include/eigen3/Eigen/Dense").exists() or \
        Path("/usr/local/include/eigen3/Eigen/Dense").exists()
    checks.append(("Eigen3 headers", eigen,
                    "run: sudo apt install libeigen3-dev"))

    ok = True
    for name, passed, hint in checks:
        status = "OK  " if passed else "MISS"
        if verbose:
            print(f"[{status}] {name}" + ("" if passed else f"  -> {hint}"))
        ok = ok and passed
    return ok


def run_colcon(packages: list[str], clean: bool, debug: bool, symlink: bool) -> int:
    if clean:
        for d in ("build", "install", "log"):
            p = WORKSPACE_ROOT / d
            if p.exists():
                print(f"Removing {p} ...")
                shutil.rmtree(p)

    build_type = "Debug" if debug else "Release"
    cmd = [
        "colcon", "build",
        "--cmake-args", f"-DCMAKE_BUILD_TYPE={build_type}",
        "--event-handlers", "console_direct+",
    ]
    if symlink:
        cmd.insert(2, "--symlink-install")
    if packages:
        cmd += ["--packages-select", *packages]

    print("Running:", " ".join(cmd))
    return subprocess.run(cmd, cwd=WORKSPACE_ROOT).returncode


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--packages", nargs="*", default=[],
                         help="Build only these packages (default: all)")
    parser.add_argument("--clean", action="store_true", help="Remove build/install/log first")
    parser.add_argument("--debug", action="store_true", help="Build with CMAKE_BUILD_TYPE=Debug")
    parser.add_argument("--no-symlink", action="store_true",
                         help="Disable --symlink-install (use for release artifacts)")
    parser.add_argument("--check-env", action="store_true",
                         help="Only run environment validation, do not build")
    parser.add_argument("--force", action="store_true",
                         help="Build even if environment validation fails")
    args = parser.parse_args()

    print("== autonomous_rc build.py ==")
    print(f"Workspace: {WORKSPACE_ROOT}")
    env_ok = check_environment()

    if args.check_env:
        return 0 if env_ok else 1

    if not env_ok and not args.force:
        print("\nEnvironment validation failed. Fix the MISS items above, or re-run with --force.")
        return 1

    return run_colcon(
        packages=args.packages,
        clean=args.clean,
        debug=args.debug,
        symlink=not args.no_symlink,
    )


if __name__ == "__main__":
    sys.exit(main())
