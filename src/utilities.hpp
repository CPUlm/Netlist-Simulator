#ifndef NETLIST_SRC_UTILITIES_HPP
#define NETLIST_SRC_UTILITIES_HPP

#include <sstream>
#include <iomanip>
#include <exception>

#include "program.hpp"

class ExitProgramNow : std::exception {};

class ReportContext;

class Utilities {
public:
  static std::string_view value_to_str(value_t v, bus_size_t size);

  template<typename T>
  static std::string to_hex_string(T v) {
    std::stringstream sstr;
    sstr << std::hex << v;
    return sstr.str();
  }

  template<typename T>
  static std::string_view to_bin_string(T v) {
    return value_to_str(v, sizeof(T));
  }

  static std::string read_file(const ReportContext &ctx, std::string_view path);
};

#endif //NETLIST_SRC_UTILITIES_HPP
