#ifndef NETLIST_SRC_UTILITIES_HPP
#define NETLIST_SRC_UTILITIES_HPP

#include "program.hpp"

class Utilities {
public:
  static std::string_view value_to_str(value_t v, bus_size_t size) noexcept;
};

#endif //NETLIST_SRC_UTILITIES_HPP
