#include "planner/raceline_loader.hpp"

#include <yaml-cpp/yaml.h>

namespace planner
{

bool RacelineLoader::loadFromYaml(const std::string & path)
{
  waypoints_.clear();
  YAML::Node root;
  try {
    root = YAML::LoadFile(path);
  } catch (const YAML::Exception &) {
    return false;
  }

  if (!root["raceline"] || !root["raceline"].IsSequence()) {
    return false;
  }

  for (const auto & node : root["raceline"]) {
    Waypoint wp;
    wp.x = node["x"].as<double>(0.0);
    wp.y = node["y"].as<double>(0.0);
    wp.target_speed_mps = node["v"].as<double>(0.0);
    waypoints_.push_back(wp);
  }
  return !waypoints_.empty();
}

}  // namespace planner
