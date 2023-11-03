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

  /// Returns the registers value.
  [[nodiscard]] virtual reg_value_t *get_registers() = 0;

  // ------------------------------------------------------
  // The simulator API
  // ------------------------------------------------------

  /// \brief Prepares the given Netlist program for simulation.
  ///
  /// This function may be used to compile the given program to machine code
  /// or do any optimizations for an interpreter.
  ///
  /// \param program The program that will be simulated.
  /// \return False in case of failure.
  virtual bool prepare(const std::shared_ptr<Program> &program) = 0;
  /// \brief Simulates a cycle of the Netlist program.
  ///
  /// How the Netlist program is effectively simulated is implementation defined.
  /// Internally, the program may be interpreted or compiled to machine code and then
  /// executed. The only thing important is that for the same program and the
  /// same inputs you should always get the same outputs.
  virtual void cycle() = 0;
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

  /// Simulates a cycle of the Netlist program.
  void cycle();

private:
  void print_register_impl(reg_t reg);

private:
  std::shared_ptr<Program> m_program;
  std::unique_ptr<SimulatorBackend> m_backend;
};

/// @}

#endif // NETLIST_SRC_SIMULATOR_HPP
