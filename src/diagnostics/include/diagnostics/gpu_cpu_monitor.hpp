// Parses `tegrastats` output (Jetson-specific system monitor) into CPU/GPU/
// RAM utilization. Pure parsing class, no ROS dependency. `tegrastats` is
// the standard way to get GPU load on Jetson (there is no `nvidia-smi` on
// Jetson's integrated GPU) but its exact output format has drifted across
// L4T/JetPack versions -- this parser is intentionally tolerant (regex-based
// field extraction, not positional) and reports `available=false` rather
// than throwing if a field is missing or the binary isn't present at all
// (e.g. running this diagnostics node on a dev laptop instead of the Orin
// Nano).
#pragma once

#include <string>

namespace diagnostics
{

struct GpuCpuSample
{
  bool available{false};
  double cpu_percent{0.0};   // average across cores reported by tegrastats
  double gpu_percent{0.0};   // GR3D_FREQ
  double ram_used_mb{0.0};
  double ram_total_mb{0.0};
};

class GpuCpuMonitor
{
public:
  // Runs `tegrastats` for a single sample via a bounded subprocess (Linux
  // only). Returns available=false (with cpu/gpu/ram left at 0) if the
  // binary is missing, the subprocess fails, or no line could be parsed.
  GpuCpuSample sample() const;

  // Exposed separately so the parsing logic can be unit-tested against a
  // captured tegrastats line without spawning a subprocess.
  static GpuCpuSample parseLine(const std::string & line);
};

}  // namespace diagnostics
