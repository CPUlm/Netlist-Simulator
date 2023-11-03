#ifndef NETLIST_SRC_INTERPRETER_SIMULATOR_HPP
#define NETLIST_SRC_INTERPRETER_SIMULATOR_HPP

#include "simulator.hpp"

// ========================================================
// class InterpreterBackend
// ========================================================

/// \ingroup simulator
/// \brief A very simple implementation of the SimulatorBackend API using a naive
/// interpreter pattern.
///
/// This is intended as a base and reference implementation of the SimulatorBackend
/// API. Moreover, it should be supported on all platforms. Also, the debugger API
/// must be supported by this implementation. In other words, the interpreter is
/// the simplest but the most complete implementation of the SimulatorBackend API.
///
/// The performance of this implementation are not a priority, still any
/// improvement in time or memory consumption is welcome.
class InterpreterBackend final : public SimulatorBackend {
public:
  InterpreterBackend();
  ~InterpreterBackend() override;

  [[nodiscard]] std::string_view get_name() const override { return "interpreter"; }

  // ------------------------------------------------------
  // The simulator API
  // ------------------------------------------------------

  [[nodiscard]] reg_value_t *get_registers() override;
  bool prepare(const std::shared_ptr<Program> &program) override;
  void cycle() override;

private:
  struct Detail;
  std::unique_ptr<Detail> m_d;
};

#endif // NETLIST_SRC_INTERPRETER_SIMULATOR_HPP
