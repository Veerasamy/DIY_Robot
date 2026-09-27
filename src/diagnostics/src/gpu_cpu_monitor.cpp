#include "diagnostics/gpu_cpu_monitor.hpp"

#include <array>
#include <cstdio>
#include <memory>
#include <regex>

namespace diagnostics
{

GpuCpuSample GpuCpuMonitor::parseLine(const std::string & line)
{
  GpuCpuSample sample;
  if (line.empty()) {return sample;}

  // RAM 2345/7620MB (lfb ...)
  std::smatch match;
  static const std::regex kRamRe(R"(RAM (\d+)/(\d+)MB)");
  if (std::regex_search(line, match, kRamRe)) {
    sample.ram_used_mb = std::stod(match[1]);
    sample.ram_total_mb = std::stod(match[2]);
    sample.available = true;
  }

  // GR3D_FREQ 45% (older) or GR3D_FREQ 45%@[...] (newer JetPack)
  static const std::regex kGpuRe(R"(GR3D_FREQ (\d+)%)");
  if (std::regex_search(line, match, kGpuRe)) {
    sample.gpu_percent = std::stod(match[1]);
    sample.available = true;
  }

  // CPU [11%@1932,8%@1932,0%@1932,...] -- average every "NN%@" occurrence.
  static const std::regex kCpuCoreRe(R"((\d+)%@)");
  auto begin = std::sregex_iterator(line.begin(), line.end(), kCpuCoreRe);
  auto end = std::sregex_iterator();
  double sum = 0.0;
  int count = 0;
  for (auto it = begin; it != end; ++it) {
    sum += std::stod((*it)[1]);
    ++count;
  }
  if (count > 0) {
    sample.cpu_percent = sum / count;
    sample.available = true;
  }

  return sample;
}

GpuCpuSample GpuCpuMonitor::sample() const
{
  // `timeout` bounds the subprocess in case tegrastats hangs or the pipe
  // never yields a line; `--count 1` is not supported on all JetPack
  // versions, so `timeout` is the primary safety net.
  const char * cmd = "timeout 2 tegrastats --interval 500 2>/dev/null";
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
  if (!pipe) {return GpuCpuSample{};}

  std::array<char, 1024> buffer{};
  std::string line;
  if (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get())) {
    line = buffer.data();
  }
  return parseLine(line);
}

}  // namespace diagnostics
