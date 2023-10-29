#ifndef NETLIST_PROGRAM_HPP
#define NETLIST_PROGRAM_HPP

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

using reg_value_t = std::uint_least64_t;
using bus_size_t = std::uint_least32_t;

/// A register name to be used in a Netlist program.
struct reg_t {
  std::uint_least32_t index = UINT_LEAST32_MAX;

  [[nodiscard]] auto operator<=>(const reg_t &) const = default;
};

struct ConstInstruction;
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

  virtual void visit_const(const ConstInstruction &inst) {}
  virtual void visit_not(const NotInstruction &inst) {}
  virtual void visit_reg(const RegInstruction &inst) {}
  virtual void visit_mux(const MuxInstruction &inst) {}
  virtual void visit_concat(const ConcatInstruction &inst) {}
  virtual void visit_and(const AndInstruction &inst) {}
  virtual void visit_nand(const NandInstruction &inst) {}
  virtual void visit_or(const OrInstruction &inst) {}
  virtual void visit_nor(const NorInstruction &inst) {}
  virtual void visit_xor(const XorInstruction &inst) {}
  virtual void visit_xnor(const XnorInstruction &inst) {}
  virtual void visit_select(const SelectInstruction &inst) {}
  virtual void visit_slice(const SliceInstruction &inst) {}
  virtual void visit_rom(const RomInstruction &inst) {}
  virtual void visit_ram(const RamInstruction &inst) {}
};

/// \addtogroup instruction The supported instructions
/// The list of all supported instructions by the Netlist parser and simulator.
/// @{

/// Base class for all instructions.
struct Instruction {
  reg_t output = {};

  Instruction() = default;
  virtual ~Instruction() = default;

  virtual void visit(ConstInstructionVisitor &visitor) const = 0;
};

/// The `output = constant` instruction.
struct ConstInstruction : Instruction {
  reg_value_t value = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_const(*this); }
};

/// The `output = NOT input` instruction.
struct NotInstruction : Instruction {
  reg_t input = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_not(*this); }
};

/// The `output = REG input` instruction.
struct RegInstruction : Instruction {
  reg_t input = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_reg(*this); }
};

/// The `output = MUX choice first second` instruction.
struct MuxInstruction : Instruction {
  reg_t choice = {};
  reg_t first = {};
  reg_t second = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_mux(*this); }
};

/// The `output = CONCAT lhs rhs` instruction.
struct ConcatInstruction : Instruction {
  reg_t lhs = {};
  reg_t rhs = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_concat(*this); }
};

/// Base class for all binary instructions such as `AND` or `XOR`.
struct BinaryInstruction : Instruction {
  reg_t lhs = {};
  reg_t rhs = {};
};

/// The `output = AND lhs rhs` instruction.
struct AndInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_and(*this); }
};

/// The `output = NAND lhs rhs` instruction.
struct NandInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_nand(*this); }
};

/// The `output = OR lhs rhs` instruction.
struct OrInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_or(*this); }
};

/// The `output = NOR lhs rhs` instruction.
struct NorInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_nor(*this); }
};

/// The `output = XOR lhs rhs` instruction.
struct XorInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_xor(*this); }
};

/// The `output = XNOR lhs rhs` instruction.
struct XnorInstruction : BinaryInstruction {
  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_xnor(*this); }
};

/// The `output = SELECT i input` instruction.
struct SelectInstruction : Instruction {
  reg_t input = {};
  bus_size_t i = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_select(*this); }
};

/// The `output = SLICE first end input` instruction.
struct SliceInstruction : Instruction {
  reg_t input = {};
  bus_size_t start = 0;
  bus_size_t end = 0;

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_slice(*this); }
};

/// The `output = ROM read_addr` instruction.
struct RomInstruction : Instruction {
  reg_t read_addr = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_rom(*this); }
};

/// The `output = RAM read_addr write_enable write_addr write_data` instruction.
struct RamInstruction : Instruction {
  reg_t read_addr = {};
  reg_t write_enable = {};
  reg_t write_addr = {};
  reg_t write_data = {};

  void visit(ConstInstructionVisitor &visitor) const override { visitor.visit_ram(*this); }
};

/// @}

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
  std::vector<Instruction *> instructions;

  ~Program() {
    for (auto *instruction : instructions)
      delete instruction;
  }

  Program() = default;
  Program(const Program &) = delete;
  Program(Program &&) noexcept = default;
};

/// The Netlist program disassembler. This class takes a program and then outputs
/// a textual representation to the given output stream.
///
/// The output is intended to contain the maximum information.
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
  struct Detail : ConstInstructionVisitor {
    std::shared_ptr<Program> program;
    std::ostream &out;

    explicit Detail(std::ostream &out) : ConstInstructionVisitor(), out(out) {}

    void print_reg(reg_t reg);
    void print_inst_label(const char *opcode, reg_t output);
    void print_binary_inst(const char *opcode, const BinaryInstruction &inst);

    void visit_const(const ConstInstruction &inst) override;
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
};

/// Utility class that generates C++ code using the ProgramBuilder API
/// to recreate the given program.
///
/// This can be quite useful to create unit tests.
///
/// **For example:**
/// If you have a program representing the following Netlist code:
/// ```
/// INPUT a
/// OUTPUT b
/// VAR a: 2, b, c
/// c = SELECT 1 a
/// b = AND a c
/// ```
/// The following C++ code will be generated:
/// ```
/// ProgramBuilder builder;
/// const auto a = builder.add_register(2, "a", 1);
/// const auto b = builder.add_register(1, "b", 2);
/// const auto c = builder.add_register(1, "c", 0);
/// builder.add_select(c, 1, a);
/// builder.add_and(b, a, c);
/// ```
class BuilderCodeGenerator {
public:
  static void generate(const std::shared_ptr<Program> &program);
  static void generate(const std::shared_ptr<Program> &program, std::ostream &out);

private:
  /// \internal
  struct Detail : ConstInstructionVisitor {
    std::shared_ptr<Program> program;
    std::ostream &out;

    explicit Detail(std::ostream &out) : ConstInstructionVisitor(), out(out) {}

    void prepare(const std::shared_ptr<Program> &program);

    [[nodiscard]] std::string get_reg_name(reg_t reg) const;

    void visit_const(const ConstInstruction &inst) override;
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
};

/// Utility class to simplify the creation of a Program instance.
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
  reg_t add_register(bus_size_t bus_size = 1, const std::string &name = {}, unsigned flags = 0);

  ConstInstruction &add_const(reg_t output, reg_value_t value);
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

  [[nodiscard]] std::shared_ptr<Program> build();

private:
  [[nodiscard]] bool check_reg(reg_t reg) const;

private:
  std::shared_ptr<Program> m_program = std::make_shared<Program>();
};

#endif
