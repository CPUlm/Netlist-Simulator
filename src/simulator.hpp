#ifndef NETLIST_SRC_SIMULATOR_HPP
#define NETLIST_SRC_SIMULATOR_HPP

#include "program.hpp"

class Simulator {
public:
  explicit Simulator(const Program &program) : m_d(program) {}

  /// Returns the current program being simulated.
  [[nodiscard]] const Program &get_program() const { return m_d.program; }

  // ------------------------------------------------------
  // The Debugger API
  // ------------------------------------------------------

  // ------------------------
  // The registers API

  /// Returns the total count of registers available (and registered in the bytecode).
  [[nodiscard]] size_t get_register_count() const { return m_d.program.registers.size(); }
  /// Returns true if the given register index is valid and refers to a real register
  /// in the current Netlist program.
  [[nodiscard]] bool is_valid_register(reg_t reg) const { return reg.index < get_register_count(); }
  /// Returns the value of the given requested \a reg. If \a reg does
  /// not exists, the behavior is undefined.
  ///
  /// The bits of the register are stored in the lowest bit of the returned value.
  [[nodiscard]] reg_value_t get_register(reg_t reg) const;
  /// Sets the value of the given requested \a reg to \a value. If \a reg
  /// does not exists, the behavior is undefined.
  ///
  /// Only the lowest bits of \a value that can be stored in \a reg are considered.
  /// Others are discarded.
  void set_register(reg_t reg, reg_value_t value);
  /// Prints the registers in the given span or all registers to the standard output.
  ///
  /// Both registers_start and registers_end are inclusives.
  ///
  /// Example, `print_registers(2, 4)` will print the 3rd, 4th and 5th
  /// registers (indices are 0-based).
  void print_registers(std::size_t registers_start = 0, std::size_t registers_end = SIZE_MAX);

  // ------------------------
  // The RAM API

  /// Prints a RAM region or all the program's RAM to the standard output.
  ///
  /// Both region_start and region_end are inclusive.
  void print_ram(std::size_t region_start = 0, std::size_t region_end = SIZE_MAX);

  // ------------------------------------------------------
  // The Simulator API
  // ------------------------------------------------------

  /// Returns true if the simulator has reached the end of the bytecode.
  [[nodiscard]] bool at_end() const { return m_d.pc >= m_d.program.instructions.size(); }
  /// Executes all instructions until a breakpoint is found. If the end
  /// of the Netlist program is reached, then restart simulating from start
  /// \a cycles - 1 count.
  ///
  /// You can call this function even if the simulator is actually stopped at
  /// a breakpoint. In that case, the execution is just resumed.
  void execute(size_t cycles = 1);
  /// Executes a single step of the simulation.
  ///
  /// You can call this function even if the simulator is actually stopped at
  /// a breakpoint. In that case, the execution is just resumed.
  void step();

private:
  struct Detail : ConstInstructionVisitor {
    explicit Detail(const Program &bytecode);

    const Program& program;
    std::size_t pc = 0;

    std::unique_ptr<reg_value_t[]> registers_value;
    std::unique_ptr<reg_value_t[]> previous_registers_value;

    void visit_const(const ConstInstruction& inst) override;
    void visit_not(const NotInstruction& inst) override;
    void visit_and(const AndInstruction& inst) override;
    void visit_nand(const NandInstruction& inst) override;
    void visit_or(const OrInstruction& inst) override;
    void visit_nor(const NorInstruction& inst) override;
    void visit_xor(const XorInstruction& inst) override;
    // TODO: void visit_xnor(const XnorInstruction& inst) override;
    void visit_reg(const RegInstruction& inst) override;
    void visit_slice(const SliceInstruction& inst) override;
    void visit_select(const SelectInstruction& inst) override;
  } m_d;
};

#endif // NETLIST_SRC_SIMULATOR_HPP
