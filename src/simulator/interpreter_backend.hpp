#ifndef NETLIST_SRC_INTERPRETER_SIMULATOR_HPP
#define NETLIST_SRC_INTERPRETER_SIMULATOR_HPP

#include "simulator.hpp"

// ========================================================
// class InterpreterBackend
// ========================================================

/// A very simple implementation of the SimulatorBackend API using a naive
/// interpreter pattern.
///
/// This is intended as a base and reference implementation of the SimulatorBackend
/// API. Moreover, it should be supported on all platforms. Also, the debugger API
/// must be supported by this implementation. In other words, the interpreter is
/// the simplest but the most complete implementation of the SimulatorBackend API.
///
/// The performance of this implementation are not a priority, still any
/// improvement in time or memory consumption is welcome.
class InterpreterBackend : public SimulatorBackend {
public:
  InterpreterBackend();
  ~InterpreterBackend() override;

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
