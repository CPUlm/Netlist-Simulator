#ifndef NETLIST_SRC_INPUT_MANAGER_HPP
#define NETLIST_SRC_INPUT_MANAGER_HPP

#include "program.hpp"

class Simulator;

// Used to parse & query inputs.
// ROM & RAM blocks are read from files, given as input on construction.
// Input Variables are read from files for each cycle or from stdin & stdout.
class InputManager {
public:
  explicit InputManager(const std::vector<std::string_view> &input_files) {}

protected:
  friend Simulator;

  std::unordered_map<ident_t, std::vector<value_t>> memory_blocks;
};

#endif //NETLIST_SRC_INPUT_MANAGER_HPP
