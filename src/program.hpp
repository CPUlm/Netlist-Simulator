#ifndef NETLIST_PROGRAM_HPP
#define NETLIST_PROGRAM_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using reg_index_t = std::uint_least32_t;
using reg_value_t = std::uint_least64_t;
using bus_size_t = std::uint_least32_t;

/// \brief A register name to be used in a Netlist program.
///
/// This is just a wrapper around a register's index that provides type safety.
struct reg_t {
  reg_index_t index = UINT_LEAST32_MAX;
  [[nodiscard]] auto operator<=>(const reg_t &) const = default;
};

struct ConstInstruction;
struct LoadInstruction;
struct NotInstruction;
struct RegInstruction;
struct MuxInstruction;
struct ConcatInstruction;
struct AndInstruction;
struct NandInstruction;
struct OrInstruction;
struct NorInstruction;
struct XorInstruction;
struct XnorInstruction;
struct SelectInstruction;
struct SliceInstruction;
struct RomInstruction;
struct RamInstruction;

/// Utility class implementing the visitor pattern for instructions.
struct ConstInstructionVisitor {
  virtual ~ConstInstructionVisitor() = default;

  virtual void visit_const(const ConstInstruction &inst) = 0;
  virtual void visit_load(const LoadInstruction &inst) = 0;
  virtual void visit_not(const NotInstruction &inst) = 0;
  virtual void visit_reg(const RegInstruction &inst) = 0;
  virtual void visit_mux(const MuxInstruction &inst) = 0;
  virtual void visit_concat(const ConcatInstruction &inst) = 0;
  virtual void visit_and(const AndInstruction &inst) = 0;
  virtual void visit_nand(const NandInstruction &inst) = 0;
  virtual void visit_or(const OrInstruction &inst) = 0;
  virtual void visit_nor(const NorInstruction &inst) = 0;
  virtual void visit_xor(const XorInstruction &inst) = 0;
  virtual void visit_xnor(const XnorInstruction &inst) = 0;
  virtual void visit_select(const SelectInstruction &inst) = 0;
  virtual void visit_slice(const SliceInstruction &inst) = 0;
  virtual void visit_rom(const RomInstruction &inst) = 0;
  virtual void visit_ram(const RamInstruction &inst) = 0;
};

/// \addtogroup instruction The supported instructions
/// The list of all supported instructions by the Netlist parser and simulator.
/// @{

/// \brief Base class for all instructions.
struct Instruction {
  reg_t output = {};

  Instruction() = default;
  virtual ~Instruction() = default;

  virtual void visit(ConstInstructionVisitor &visitor) const = 0;
};

/// \brief The `output = constant` instruction.
struct ConstInstruction : Instruction {
  reg_value_t value = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_const(*this); }
};

/// \brief The `output = input` instruction.
struct LoadInstruction : Instruction {
  reg_t input = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_load(*this); }
};

/// \brief The `output = NOT input` instruction.
struct NotInstruction : Instruction {
  reg_t input = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_not(*this); }
};

/// \brief The `output = REG input` instruction.
struct RegInstruction : Instruction {
  reg_t input = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_reg(*this); }
};

/// \brief The `output = MUX choice first second` instruction.
struct MuxInstruction : Instruction {
  reg_t choice = {};
  reg_t first = {};
  reg_t second = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_mux(*this); }
};

/// \brief The `output = CONCAT lhs rhs` instruction.
struct ConcatInstruction : Instruction {
  reg_t lhs = {};
  reg_t rhs = {};
  // How many bits should RHS be shifted? This corresponds to the bus size of LHS.
  bus_size_t offset = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_concat(*this); }
};

/// \brief Base class for all binary instructions such as `AND` or `XOR`.
struct BinaryInstruction : Instruction {
  reg_t lhs = {};
  reg_t rhs = {};
};

/// \brief The `output = AND lhs rhs` instruction.
struct AndInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_and(*this); }
};

/// \brief The `output = NAND lhs rhs` instruction.
struct NandInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_nand(*this); }
};

/// \brief The `output = OR lhs rhs` instruction.
struct OrInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_or(*this); }
};

/// \brief The `output = NOR lhs rhs` instruction.
struct NorInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_nor(*this); }
};

/// \brief The `output = XOR lhs rhs` instruction.
struct XorInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_xor(*this); }
};

/// \brief The `output = XNOR lhs rhs` instruction.
struct XnorInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_xnor(*this); }
};

/// \brief The `output = SELECT i input` instruction.
struct SelectInstruction : Instruction {
  reg_t input = {};
  bus_size_t i = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_select(*this); }
};

/// \brief The `output = SLICE first end input` instruction.
struct SliceInstruction : Instruction {
  reg_t input = {};
  bus_size_t start = 0;
  bus_size_t end = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_slice(*this); }
};

/// \brief Base class for `ROM` and `RAM` instructions.
struct MemoryInstruction : Instruction {
  /// An index inside Program::memories array.
  std::uint_least32_t memory_block = 0;
};

/// \brief The `output = ROM read_addr` instruction.
struct RomInstruction : MemoryInstruction {
  reg_t read_addr = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_rom(*this); }
};

/// \brief The `output = RAM addr_size word_size read_addr write_enable write_addr write_data` instruction.
struct RamInstruction : MemoryInstruction {
  reg_t read_addr = {};
  reg_t write_enable = {};
  reg_t write_addr = {};
  reg_t write_data = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_ram(*this); }
};

/// @}

/// Meta information about a RAM or ROM memory block.
struct MemoryInfo {
  /// The parent instruction (RAM or ROM) to who belongs this memory block.
  Instruction *parent = nullptr;
  bus_size_t addr_size = 0;
  bus_size_t word_size = 0;

  /// Returns the memory total size in count of words.
  [[nodiscard]] size_t get_size() const { return 1 << addr_size; }
};

/// Possible flags for a register.
/// \see RegisterInfo
enum RegisterInfoFlag {
  RIF_NONE = 0x0,
  /// The register represents an input.
  RIF_INPUT = 0x1,
  /// The register represents an output.
  RIF_OUTPUT = 0x2,
  /// The register is an internal register used by the parser to implement
  /// some functionalities. It doesn't correspond to a variable declared
  /// in the `VAR` statement.
  RIF_INTERNAL = 0x4,
};

/// Meta information about a program's register.
struct RegisterInfo {
  /// The register's name (for debugging purposes). If the name is unknown,
  /// an empty string can be used.
  std::string name = {};
  /// The size of the register. Must be in the range [1,64].
  bus_size_t bus_size = 1;
  /// \see RegisterInfoFlag
  unsigned flags = RIF_NONE;
};

/// A Netlist program represented by a sequence of instructions to be simulated and a set of registers.
struct Program {
  std::vector<RegisterInfo> registers;
  std::vector<MemoryInfo> memories;
  std::vector<Instruction *> instructions;

  ~Program() {
    for (auto *instruction : instructions)
      delete instruction;
  }

  Program() = default;
  Program(const Program &) = delete;
  Program(Program &&) noexcept = default;

  /// \brief Returns \c true if the program is empty, that is if it doesn't have any instruction.
  [[nodiscard]] bool is_empty() const { return instructions.empty(); }

  /// \brief Returns \c true if the program has at least one input.
  [[nodiscard]] bool has_inputs() const;
  /// \brief Returns the inputs of the program.
  [[nodiscard]] std::vector<reg_t> get_inputs() const;

  /// \brief Returns \c true if the program has at least one output.
  [[nodiscard]] bool has_outputs() const;
  /// \brief Returns the outputs of the program.
  [[nodiscard]] std::vector<reg_t> get_outputs() const;

  /// \brief Returns the register's name.
  ///
/// \brief The Netlist program disassembler.
///
/// This class takes a program and then outputs a textual representation
/// to the given output stream.
///
/// The output is intended to contain the maximum information and to be a valid
/// parseable Netlist program.
class Disassembler {
public:
  /// Disassembles a single instruction and prints it to std::cout.
  static void disassemble(const Instruction &instruction);
  /// Disassembles a single instruction and prints to the given output stream.
  static void disassemble(const Instruction &instruction, std::ostream &out);

  /// Disassembles all the program and prints it to std::cout.
  static void disassemble(const std::shared_ptr<Program> &program);
  /// Disassembles all the program and prints it to the given output stream.
  static void disassemble(const std::shared_ptr<Program> &program, std::ostream &out);

private:
  /// \internal
  struct Detail final : ConstInstructionVisitor {
    std::shared_ptr<Program> program;
    std::ostream &out;

    explicit Detail(std::ostream &out) : ConstInstructionVisitor(), out(out) {}

    void print_reg(reg_t reg);
    void print_inst_label(const char *opcode, reg_t output);
    void print_binary_inst(const char *opcode, const BinaryInstruction &inst);

    void visit_const(const ConstInstruction &inst) override;
    void visit_load(const LoadInstruction &inst) override;
    void visit_not(const NotInstruction &inst) override;
    void visit_reg(const RegInstruction &inst) override;
    void visit_mux(const MuxInstruction &inst) override;
    void visit_concat(const ConcatInstruction &inst) override;
    void visit_and(const AndInstruction &inst) override;
    void visit_nand(const NandInstruction &inst) override;
    void visit_or(const OrInstruction &inst) override;
    void visit_nor(const NorInstruction &inst) override;
    void visit_xor(const XorInstruction &inst) override;
    void visit_xnor(const XnorInstruction &inst) override;
    void visit_select(const SelectInstruction &inst) override;
    void visit_slice(const SliceInstruction &inst) override;
    void visit_rom(const RomInstruction &inst) override;
    void visit_ram(const RamInstruction &inst) override;
  };
  /// If the register has a name then it is returned, otherwise a dummy but
  /// valid identifier is returned uniquely identifying the register.
  [[nodiscard]] std::string get_register_name(reg_t reg) const;
};

/// \brief Utility class to simplify the creation of a Program instance.
///
/// To create an instance of Program representing the following Netlist code:
/// ```
/// INPUT a, b
/// OUTPUT c, s
/// VAR a, b, c, s
/// c = AND a b
/// s = XOR a b
/// ```
/// You can use the following C++ code:
/// ```
/// ProgramBuilder builder;
/// const reg_t a = builder.add_register(1, "a", RIF_INPUT);
/// const reg_t b = builder.add_register(1, "b", RIF_INPUT);
/// const reg_t c = builder.add_register(1, "c", RIF_OUTPUT);
/// const reg_t s = builder.add_register(1, "d", RIF_OUTPUT);
/// builder.add_and(c, a, b);
/// builder.add_xor(s, a, b);
/// std::shared_ptr<Program> program = builder.build();
/// ```
class ProgramBuilder {
public:
  [[nodiscard]] reg_t add_register(bus_size_t bus_size = 1, const std::string &name = {}, unsigned flags = 0);
  [[nodiscard]] bus_size_t get_register_bus_size(reg_t reg) const;

  ConstInstruction &add_const(reg_t output, reg_value_t value);
  LoadInstruction &add_load(reg_t output, reg_t input);
  NotInstruction &add_not(reg_t output, reg_t input);
  AndInstruction &add_and(reg_t output, reg_t lhs, reg_t rhs);
  NandInstruction &add_nand(reg_t output, reg_t lhs, reg_t rhs);
  OrInstruction &add_or(reg_t output, reg_t lhs, reg_t rhs);
  NorInstruction &add_nor(reg_t output, reg_t lhs, reg_t rhs);
  XorInstruction &add_xor(reg_t output, reg_t lhs, reg_t rhs);
  XnorInstruction &add_xnor(reg_t output, reg_t lhs, reg_t rhs);
  ConcatInstruction &add_concat(reg_t output, reg_t lhs, reg_t rhs);
  RegInstruction &add_reg(reg_t output, reg_t input);
  MuxInstruction &add_mux(reg_t output, reg_t choice, reg_t first, reg_t second);
  SelectInstruction &add_select(reg_t output, bus_size_t i, reg_t input);
  SliceInstruction &add_slice(reg_t output, bus_size_t start, bus_size_t end, reg_t input);
  RomInstruction &add_rom(reg_t output, bus_size_t addr_size, bus_size_t word_size, reg_t read_addr);
  RamInstruction &add_ram(reg_t output, bus_size_t addr_size, bus_size_t word_size, reg_t read_addr, reg_t write_enable,
                          reg_t write_addr, reg_t write_data);

  [[nodiscard]] std::shared_ptr<Program> build();

private:
  [[nodiscard]] bool check_reg(reg_t reg) const;

private:
  std::shared_ptr<Program> m_program = std::make_shared<Program>();
};

#endif
