#ifndef NETLIST_SRC_INPUT_MANAGER_HPP
#define NETLIST_SRC_INPUT_MANAGER_HPP

#include "program.hpp"

class Simulator;

// Used to parse & query inputs.
// ROM & RAM blocks are read from files, given as input on construction.
// Input Variables are read from files for each cycle or from stdin & stdout.
class InputManager {
public:
  explicit InputManager();
  // TODO : Build Class.

protected:
  friend Simulator;
  value_t get_input_value(ident_t var_name);

  std::unordered_map<ident_t, std::vector<value_t>> memory_blocks;
  void register_input_variables(const std::unordered_set<Variable::ptr> &input_var);
};

#endif //NETLIST_SRC_INPUT_MANAGER_HPP
