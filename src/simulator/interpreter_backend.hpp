#ifndef NETLIST_SRC_INTERPRETER_SIMULATOR_HPP
#define NETLIST_SRC_INTERPRETER_SIMULATOR_HPP

#include "simulator.hpp"

class InterpreterBackend : public SimulatorBackend {
public:
  InterpreterBackend();
  ~InterpreterBackend();

  [[nodiscard]] std::string_view get_name() const override { return "interpreter"; }

  // ------------------------------------------------------
  // The simulator API
  // ------------------------------------------------------

  bool prepare(const std::shared_ptr<Program> &program) override;
  void simulate(reg_value_t *inputs, reg_value_t *outputs, size_t n) override;

  // ------------------------------------------------------
  // The debugger API
  // ------------------------------------------------------

  /// The interpreter backend supports the debugger API !
  [[nodiscard]] bool has_debugger() const override { return true; }

  [[nodiscard]] reg_value_t get_register(reg_t reg) const override;
  void set_register(reg_t reg, reg_value_t value) override;

  void step() override;

private:
  struct Detail;
  std::unique_ptr<Detail> m_d;
};

#endif // NETLIST_SRC_INTERPRETER_SIMULATOR_HPP
