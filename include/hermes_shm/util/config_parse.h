/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Distributed under BSD 3-Clause license.                                   *
 * Copyright by The HDF Group.                                               *
 * Copyright by the Illinois Institute of Technology.                        *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of Hermes. The full Hermes copyright notice, including  *
 * terms governing use, modification, and redistribution, is contained in    *
 * the COPYING file, which can be found at the top directory. If you do not  *
 * have access to the file, you may request a copy from help@hdfgroup.org.   *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef HSHM_CONFIG_PARSE_PARSER_H
#define HSHM_CONFIG_PARSE_PARSER_H

#include <float.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include <cstdlib>
#include <iomanip>
#include <list>
#include <regex>
#include <string>

#include "formatter.h"
#include "hermes_shm/constants/macros.h"
#include "logging.h"
#include "yaml-cpp/yaml.h"

namespace hshm {

class ConfigParse {
 public:
  static void rm_char(std::string &str, char ch) {
    str.erase(std::remove(str.begin(), str.end(), ch), str.end());
  }

  /**
   * parse a hostfile string
   * [] represents a range to generate
   * ; represents a new host name completely
   *
   * Example: hello[00-09,10]-40g;hello2[11-13]-40g
   * */
  static void ParseHostNameString(std::string hostname_set_str,
                                  std::vector<std::string> &list) {
    // Remove all whitespace characters from host name
    rm_char(hostname_set_str, ' ');
    rm_char(hostname_set_str, '\n');
    rm_char(hostname_set_str, '\r');
    rm_char(hostname_set_str, '\t');
    if (hostname_set_str.size() == 0) {
      return;
    }
    // Expand hostnames
    std::stringstream ss(hostname_set_str);
    while (ss.good()) {
      // Get the current host
      std::string hostname;
      std::getline(ss, hostname, ';');

      // Divide the hostname string into prefix, ranges, and suffix
      auto lbracket = hostname.find_first_of('[');
      auto rbracket = hostname.find_last_of(']');
      std::string prefix, ranges_str, suffix;
      if (lbracket != std::string::npos && rbracket != std::string::npos) {
        /*
         * For example, hello[00-09]-40g
         * lbracket = 5
         * rbracket = 11
         * prefix = hello (length: 5)
         * range = 00-09 (length: 5)
         * suffix = -40g (length: 4)
         * */
        prefix = hostname.substr(0, lbracket);
        ranges_str = hostname.substr(lbracket + 1, rbracket - lbracket - 1);
        suffix = hostname.substr(rbracket + 1);
      } else {
        list.emplace_back(hostname);
        continue;
      }

      // Parse the range list into a tuple of (min, max, num_width)
      std::stringstream ss_ranges(ranges_str);
      std::vector<std::tuple<int, int, int>> ranges;
      while (ss_ranges.good()) {
        // Parse ',' and remove spaces
        std::string range_str;
        std::getline(ss_ranges, range_str, ',');
        rm_char(range_str, ' ');

        // Divide the range by '-'
        auto dash = range_str.find_first_of('-');
        if (dash != std::string::npos) {
          int min, max;
          // Get the minimum and maximum value
          std::string min_str = range_str.substr(0, dash);
          std::string max_str = range_str.substr(dash + 1);
          std::stringstream(min_str) >> min;
          std::stringstream(max_str) >> max;

          // Check for leading 0s
          int num_width = 0;
          if (min_str.size() == max_str.size()) {
            num_width = min_str.size();
          }

          // Place the range with width
          ranges.emplace_back(min, max, num_width);
        } else if (range_str.size()) {
          int val;
          std::stringstream(range_str) >> val;
          ranges.emplace_back(val, val, range_str.size());
        }
      }

      // Expand the host names by each range
      for (auto &range : ranges) {
        int min = std::get<0>(range);
        int max = std::get<1>(range);
        int num_width = std::get<2>(range);

        for (int i = min; i <= max; ++i) {
          std::stringstream host_ss;
          host_ss << prefix;
          host_ss << std::setw(num_width) << std::setfill('0') << i;
          host_ss << suffix;
          list.emplace_back(host_ss.str());
        }
      }
    }
  }

  /** parse the suffix of \a num_text NUMBER text */
  static std::string ParseNumberSuffix(const std::string &num_text) {
    size_t i;
    for (i = 0; i < num_text.size(); ++i) {
      char c = num_text[i];
      // Skip numbers
      if ('0' <= c && c <= '9') {
        continue;
      }
      // Skip whitespace
      if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
        continue;
      }
      // Skip period (for floats)
      if (c == '.') {
        continue;
      }
      break;
    }
    return std::string(num_text.begin() + i, num_text.end());
    ;
  }

  /** parse the number of \a num_text NUMBER text */
  template <typename T>
  static T ParseNumber(const std::string &num_text) {
    T size;
    
    size_t i = 0;
    std::string numericPart;

    if (num_text == "inf") {
      return std::numeric_limits<T>::max();
    }
    
    // This is necessary for AppleClang. It can't handle "PB" unit.
    while (i < num_text.length() && (std::isdigit(num_text[i]) || num_text[i] == '.')) {
      numericPart += num_text[i];
      i++;
    }
    
    std::stringstream(numericPart) >> size;
    return size;
  }

  /** Converts \a size_text SIZE text into a size_t */
  static hshm::u64 ParseSize(const std::string &size_text) {
    auto size = ParseNumber<double>(size_text);
    if (size_text == "inf") {
      return std::numeric_limits<hshm::u64>::max();
    }
    std::string suffix = ParseNumberSuffix(size_text);
    
    if (suffix.empty()) {
      return Unit<hshm::u64>::Bytes(size);
    } else if (suffix[0] == 'k' || suffix[0] == 'K') {
      return hshm::Unit<hshm::u64>::Kilobytes(size);
    } else if (suffix[0] == 'm' || suffix[0] == 'M') {
      return hshm::Unit<hshm::u64>::Megabytes(size);
    } else if (suffix[0] == 'g' || suffix[0] == 'G') {
      return hshm::Unit<hshm::u64>::Gigabytes(size);
    } else if (suffix[0] == 't' || suffix[0] == 'T') {
      return hshm::Unit<hshm::u64>::Terabytes(size);
    } else if (suffix[0] == 'p' || suffix[0] == 'P') {
      return hshm::Unit<hshm::u64>::Petabytes(size);
    } else {
      HELOG(kFatal, "Could not parse the size: {}", size_text);
      exit(1);
    }
  }

  /** Returns bandwidth (bytes / second) */
  static hshm::u64 ParseBandwidth(const std::string &size_text) {
    return ParseSize(size_text);
  }

  /** Returns latency (nanoseconds) */
  static hshm::u64 ParseLatency(const std::string &latency_text) {
    auto size = ParseNumber<double>(latency_text);
    std::string suffix = ParseNumberSuffix(latency_text);
    if (suffix.empty()) {
      return Unit<hshm::u64>::Bytes(size);
    } else if (suffix[0] == 'n' || suffix[0] == 'N') {
      return Unit<hshm::u64>::Bytes(size);
    } else if (suffix[0] == 'u' || suffix[0] == 'U') {
      return hshm::Unit<hshm::u64>::Kilobytes(size);
    } else if (suffix[0] == 'm' || suffix[0] == 'M') {
      return hshm::Unit<hshm::u64>::Megabytes(size);
    } else if (suffix[0] == 's' || suffix[0] == 'S') {
      return hshm::Unit<hshm::u64>::Terabytes(size);
    }
    HELOG(kFatal, "Could not parse the latency: {}", latency_text);
    return 0;
  }

  /** Expands all environment variables in a path string */
  static std::string ExpandPath(std::string path) {
    std::smatch env_names;
    std::regex expr("\\$\\{[^\\}]+\\}");
    if (!std::regex_search(path, env_names, expr)) {
      return path;
    }
    for (auto &env_name_re : env_names) {
      std::string to_replace = std::string(env_name_re);
      std::string env_name = to_replace.substr(2, to_replace.size() - 3);
      std::string env_val = hshm::SystemInfo::Getenv(
          env_name.c_str(), hshm::Unit<size_t>::Megabytes(1));
      std::regex replace_expr("\\$\\{" + env_name + "\\}");
      path = std::regex_replace(path, replace_expr, env_val);
    }
    return path;
  }

  /** Parse hostfile */
  static std::vector<std::string> ParseHostfile(const std::string &path) {
    std::vector<std::string> hosts;
    std::ifstream file(path);
    if (file.is_open()) {
      std::string line;
      while (std::getline(file, line)) {
        hshm::ConfigParse::ParseHostNameString(line, hosts);
      }
      file.close();
    } else {
      HELOG(kError, "Could not open the hostfile: {}", path);
    }
    return hosts;
  }
};

/**
 * Base class for configuration files
 * */
class BaseConfig {
 public:
  /** load configuration from a string */
  void LoadText(const std::string &config_string, bool with_default = true) {
    if (with_default) {
      LoadDefault();
    }
    if (config_string.size() == 0) {
      return;
    }
    YAML::Node yaml_conf = YAML::Load(config_string);
    ParseYAML(yaml_conf);
  }

  /** load configuration from file */
  void LoadFromFile(const std::string &path, bool with_default = true) {
    if (with_default) {
      LoadDefault();
    }
    if (path.size() == 0) {
      return;
    }
    auto real_path = hshm::ConfigParse::ExpandPath(path);
    try {
      YAML::Node yaml_conf = YAML::LoadFile(real_path);
      ParseYAML(yaml_conf);
    } catch (std::exception &e) {
      HELOG(kFatal, e.what());
    }
  }

  /** load the default configuration */
  virtual void LoadDefault() = 0;

 public:
  /** parse \a list_node vector from configuration file in YAML */
  template <typename T, typename VEC_TYPE = std::vector<T>>
  static void ParseVector(YAML::Node list_node, VEC_TYPE &list) {
    for (auto val_node : list_node) {
      list.emplace_back(val_node.as<T>());
    }
  }

  /** clear + parse \a list_node vector from configuration file in YAML */
  template <typename T, typename VEC_TYPE = std::vector<T>>
  static void ClearParseVector(YAML::Node list_node, VEC_TYPE &list) {
    list.clear();
    for (auto val_node : list_node) {
      list.emplace_back(val_node.as<T>());
    }
  }

 private:
  virtual void ParseYAML(YAML::Node &yaml_conf) = 0;
};

}  // namespace hshm

#endif  // HSHM_CONFIG_PARSE_PARSER_H
