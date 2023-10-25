#ifndef NETLIST_SRC_SIMULATOR_HPP
#define NETLIST_SRC_SIMULATOR_HPP

#include "bytecode.hpp"

/// Represents the data of a breakpoint inside a Netlist bytecode. This is a critical
/// data piece used to implement and interact with the Netlist debugger.
struct BreakPoint {
  /// The offset into the bytecode where this breakpoint is located. It is an index
  /// into the bytecode's words array.
  std::size_t offset = 0;
  /// Breakpoint are implemented using a special instruction. The debugger patch the
  /// bytecode by replacing an instruction by a breakpoint instruction. However, when
  /// it is time to resume the execution, the previous bytecode must be recovered.
  /// Because a breakpoint instruction is always encoded as a single word, we only need
  /// store a single word to fully recover the previous bytecode (before the breakpoint
  /// being added).
  ///
  /// As a matter of fact, breakpoints in the Netlist simulator are implemented similarly
  /// as real breakpoints in x86 or ARM code.
  std::uint32_t saved_word = 0;
  /// Is the breakpoint oneshot? If true, then the breakpoint is automatically removed
  /// once reached.
  bool oneshot = false;
  /// Is the breakpoint currently active? That is the underlying bytecode patched to
  /// detect this breakpoint or not.
  /// Not active breakpoints are ignored by the simulator.
  bool is_active = false;

  void activate(ByteCode &bytecode);
  void deactivate(ByteCode &bytecode);
};

/// The Netlist simulator / virtual machine.
///
/// Actually, the simulator is implemented as a very small virtual machine that
/// takes a list of instructions (the Netlist bytecode) and interprets each instruction
/// step by step. The actual implementation of each instruction is done inside the
/// private execute_*() functions.
///
/// Because the debugger is really simple, it is implemented inside the simulator itself.
/// Therefore, the debugger API is part of the Simulator API. See the add_breakpoint(),
/// print_registers(), print_ram(), and like, functions. Likewise, the execute() and step()
/// functions are fully aware of breakpoints.
class Simulator {
public:
  Simulator(const ByteCode &bytecode) : m_d(bytecode) {}

  /// Returns the current bytecode being simulated.
  [[nodiscard]] ByteCode &get_bytecode() { return m_d.bytecode; }
  [[nodiscard]] const ByteCode &get_bytecode() const { return m_d.bytecode; }

  // ------------------------------------------------------
  // The Debugger API
  // ------------------------------------------------------

  // ------------------------
  // The breakpoints API

  /// Returns true if the simulator is stopped at a breakpoint in the bytecode.
  ///
  /// If it is the case, you can resume the execution using the execute() function or
  /// the step() depending on what you want. You can also inspect the execution of
  /// the program using any of the debugger API (e.g. print_registers(), etc.).
  [[nodiscard]] bool at_breakpoint() const { return m_d.at_breakpoint; }
  /// Returns the total count of registered breakpoints (active or not).
  [[nodiscard]] size_t get_breakpoint_count() const { return m_d.breakpoints.size(); }

  // ------------------------
  // The registers API

  /// Returns the total count of registers available (and registered in the bytecode).
  [[nodiscard]] size_t get_register_count() const { return m_d.bytecode.registers.size(); }
  /// Returns true if the given register index is valid and refers to a real register
  /// in the current Netlist program.
  [[nodiscard]] bool is_valid_register(RegIndex reg) const { return reg < get_register_count(); }
  /// Returns the value of the given requested \a reg. If \a reg does
  /// not exists, the behavior is undefined.
  ///
  /// The bits of the register are stored in the lowest bit of the returned value.
  [[nodiscard]] RegValue get_register(RegIndex reg) const;
  /// Sets the value of the given requested \a reg to \a value. If \a reg
  /// does not exists, the behavior is undefined.
  ///
  /// Only the lowest bits of \a value that can be stored in \a reg are considered.
  /// Others are discarded.
  void set_register(RegIndex reg, RegValue value);
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
  [[nodiscard]] bool at_end() const { return m_d.at_end(); }
  /// Executes all instructions until a breakpoint is found.
  /// If there is no breakpoint, then totally simulate the Netlist.
  ///
  /// You can call this function even if the simulator is actually stopped at
  /// a breakpoint. In that case, the execution is just resumed.
  void execute();
  /// Executes a single step of the simulation.
  ///
  /// You can call this function even if the simulator is actually stopped at
  /// a breakpoint. In that case, the execution is just resumed.
  void step();

private:
  /// Finds the breakpoint that is located at the given position in the bytecode.
  /// If there is no breakpoint at the location, then nullptr is returned.
  std::vector<BreakPoint>::iterator find_breakpoint(std::size_t pc);
  void handle_breakpoint();

  struct Detail : public ByteCodeReader<Detail> {
    explicit Detail(const ByteCode &bytecode);

    ByteCode bytecode;
    std::vector<BreakPoint> breakpoints;
    std::unique_ptr<RegValue[]> registers_value;
    std::unique_ptr<RegValue[]> previous_registers_value;
    bool at_breakpoint = false;

    void handle_nop() {}
    void handle_break();
    void handle_const(const ConstInstruction &inst);
    void handle_not(const NotInstruction &inst);
    void handle_binary_inst(const char *name, const BinaryInstruction &inst);
    void handle_and(const BinaryInstruction &inst);
    void handle_or(const BinaryInstruction &inst);
    void handle_nand(const BinaryInstruction &inst);
    void handle_nor(const BinaryInstruction &inst);
    void handle_xor(const BinaryInstruction &inst);
    void handle_reg(const RegInstruction &inst);
    void handle_slice(const SliceInstruction &inst);
    void handle_select(const SelectInstruction &inst);
  } m_d;
};

#endif // NETLIST_SRC_SIMULATOR_HPP
