#ifndef NETLIST_SRC_SIMULATOR_HPP
#define NETLIST_SRC_SIMULATOR_HPP

#include "program.hpp"

#include <cassert>

/// \addtogroup simulator The simulator
/// @{

// ========================================================
// class SimulatorBackend
// ========================================================

/// \brief The interface for Netlist simulator's backends.
///
/// To actually call the simulator, please use the more complete and user-friendly
/// Simulator interface.
///
/// \see Simulator
class SimulatorBackend {
public:
  virtual ~SimulatorBackend() = default;

  /// Returns the backend name.
  [[nodiscard]] virtual std::string_view get_name() const = 0;

  // ------------------------------------------------------
  // The simulator API
  // ------------------------------------------------------

  /// \brief Prepares the given Netlist program for further simulation.
  ///
  /// This function may be used to compile the given program to machine code
  /// or do any optimizations for an interpreter.
  ///
  /// \param program
  /// \return False in case of failure.
  virtual bool prepare(const std::shared_ptr<Program> &program) = 0;
  /// \brief Simulates a Netlist program.
  ///
  /// How the Netlist program is effectively simulated is implementation defined.
  /// Internally, the program may be interpreted or compiled to machine code and then
  /// executed. The only thing important is that for the same program and the
  /// same inputs you should always get the same outputs.
  ///
  /// \param inputs The inputs of the Netlist, in the same order as they were found in the program.
  /// \param outputs The outputs of the Netlist, in the same order as they were found in the program.
  /// \param n The count of cycles to simulate. By default is 1 cycle.
  virtual void simulate(reg_value_t *inputs, reg_value_t *outputs, size_t n = 1) = 0;

  // ------------------------------------------------------
  // The debugger API
  // ------------------------------------------------------

  /// \brief Does this backend support the debugger API of the simulator?
  ///
  /// By default, this function returns false.
  [[nodiscard]] virtual bool has_debugger() const { return false; }

  /// \brief Returns the value of the given requested \a reg.
  ///
  /// \warning This function only works if has_debugger() returns true.
  ///
  /// \param reg The register to request. If \a reg does not exists, the behavior is undefined.
  /// \return The bits of the requested register stored in the lowest bit of the returned value.
  [[nodiscard]] virtual reg_value_t get_register(reg_t reg) const { assert(false && "debugger API not supported"); return 0; }
  /// \brief Sets the value of the given requested \a reg to \a value.
  ///
  /// \warning This function only works if has_debugger() returns true.
  ///
  /// \param reg The register to update. If \a reg does not exists, the behavior is undefined.
  /// \param value The new register's value. Only the lowest bits of \a value that can be
  ///              stored in \a reg are considered. Others are discarded.
  virtual void set_register(reg_t reg, reg_value_t value) { assert(false && "debugger API not supported"); }

  /// \brief Simulates a single instruction in the Netlist program.
  ///
  /// \warning This function only works if has_debugger() returns true.
  virtual void step() { assert(false && "debugger API not supported"); }
};

// ========================================================
// class Simulator
// ========================================================

/// \brief The Netlist simulator interface.
///
/// The actual simulator logic is implemented inside an implementation of the
/// interface SimulatorBackend.
///
/// \see SimulatorBackend
class Simulator {
public:
  explicit Simulator(const std::shared_ptr<Program> &program);

  /// \brief Returns the current program being simulated.
  [[nodiscard]] std::shared_ptr<Program> get_program() const { return m_program; }

  // ------------------------------------------------------
  // The debugger API
  // ------------------------------------------------------

  // ------------------------
  // The registers API

  /// \brief Returns the total count of registers available (and registered in the bytecode).
  [[nodiscard]] size_t get_register_count() const { return m_program->registers.size(); }
  /// \brief Returns true if the given register index is valid and refers to a real register
  /// in the current Netlist program.
  [[nodiscard]] bool is_valid_register(reg_t reg) const { return reg.index < get_register_count(); }
  /// \brief Shorthand for `get_backend()->get_register(reg)`.
  [[nodiscard]] reg_value_t get_register(reg_t reg) const;
  /// \brief Shorthand for `get_backend()->set_register(reg, value)`.
  void set_register(reg_t reg, reg_value_t value);
  /// \brief Prints the given register value to the standard output.
  ///
  /// \warning This need debugger support from the backend.
  ///
  /// \param reg The register to print, behavior is undefined if the register doesn't exist.
  void print_register(reg_t reg);
  /// \brief Prints the registers in the given range to the standard output.
  ///
  /// \warning This need debugger support from the backend.
  ///
  /// \param registers_start The first register index to print, inclusive.
  /// \param registers_end The last register index to print, inclusive.
  void print_registers(std::uint_least32_t registers_start = 0, std::uint_least32_t registers_end = UINT_LEAST32_MAX);

  /// \brief Prints the inputs to the standard output.
  ///
  /// \warning This need debugger support from the backend.
  void print_inputs();
  /// \brief Prints the outputs to the standard output.
  ///
  /// \warning This need debugger support from the backend.
  void print_outputs();

  // ------------------------------------------------------
  // The Simulator API
  // ------------------------------------------------------

  /// Simulates the Netlist program for \a n cycles.
  ///
  /// You can call this function even if the simulator is actually stopped at
  /// a breakpoint. In that case, the execution is just resumed.
  void execute(size_t n = 1);
  /// \brief Executes a single step of the simulation.
  ///
  /// \warning This function only works if the currently used backend supports
  /// the debugger API.
  ///
  /// You can call this function even if the simulator is actually stopped at
  /// a breakpoint. In that case, the execution is just resumed.
  void step();

private:
  void print_register_impl(reg_t reg);

private:
  std::shared_ptr<Program> m_program;
  std::unique_ptr<SimulatorBackend> m_backend;
};

/// @}

#endif // NETLIST_SRC_SIMULATOR_HPP
