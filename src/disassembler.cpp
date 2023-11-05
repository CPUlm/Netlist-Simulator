#include "disassembler.hpp"

#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <ostream>

struct DisassemblerVisitor final : ConstInstructionVisitor {
  std::shared_ptr<Program> context;
  std::ostream &out;

  explicit DisassemblerVisitor(const std::shared_ptr<Program> &ctx, std::ostream &out)
      : ConstInstructionVisitor(), context(ctx), out(out) {}

  void print_binary_instruction(const char *opcode, const BinaryInstruction &inst) {
    const auto output = context->get_register_name(inst.output);
    const auto lhs = context->get_register_name(inst.lhs);
    const auto rhs = context->get_register_name(inst.rhs);
    out << fmt::format("{} = {} {} {}", output, opcode, lhs, rhs);
  }

  void visit_const(const ConstInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto bus_size = context->registers[inst.output.index].bus_size;
    out << fmt::format("{} = {:0{}b}", output, inst.value, bus_size);
  }

  void visit_load(const LoadInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto input = context->get_register_name(inst.input);
    out << fmt::format("{} = {}", output, input);
  }

  void visit_not(const NotInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto input = context->get_register_name(inst.input);
    out << fmt::format("{} = NOT {}", output, input);
  }

  void visit_reg(const RegInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto input = context->get_register_name(inst.input);
    out << fmt::format("{} = REG {}", output, input);
  }

  void visit_mux(const MuxInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto choice = context->get_register_name(inst.choice);
    const auto first = context->get_register_name(inst.first);
    const auto second = context->get_register_name(inst.second);
    out << fmt::format("{} = MUX {} {} {}", output, choice, first, second);
  }

  void visit_concat(const ConcatInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto lhs = context->get_register_name(inst.lhs);
    const auto rhs = context->get_register_name(inst.rhs);
    out << fmt::format("{} = CONCAT {} {}", output, lhs, rhs);
  }

  void visit_and(const AndInstruction &inst) override { print_binary_instruction("AND", inst); }

  void visit_nand(const NandInstruction &inst) override { print_binary_instruction("NAND", inst); }

  void visit_or(const OrInstruction &inst) override { print_binary_instruction("OR", inst); }

  void visit_nor(const NorInstruction &inst) override { print_binary_instruction("NOR", inst); }

  void visit_xor(const XorInstruction &inst) override { print_binary_instruction("XOR", inst); }

  void visit_xnor(const XnorInstruction &inst) override { print_binary_instruction("XNOR", inst); }

  void visit_select(const SelectInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto input = context->get_register_name(inst.input);
    out << fmt::format("{} = SELECT {} {}", output, inst.i, input);
  }

  void visit_slice(const SliceInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto input = context->get_register_name(inst.input);
    out << fmt::format("{} = SLICE {} {} {}", output, inst.start, inst.end, input);
  }

  void visit_rom(const RomInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto read_addr = context->get_register_name(inst.read_addr);
    const auto &memory_info = context->memories[inst.memory_block];
    out << fmt::format("{} = ROM {} {} {}", output, memory_info.addr_size, memory_info.word_size, read_addr);
  }

  void visit_ram(const RamInstruction &inst) override {
    const auto output = context->get_register_name(inst.output);
    const auto read_addr = context->get_register_name(inst.read_addr);
    const auto write_enable = context->get_register_name(inst.write_enable);
    const auto write_addr = context->get_register_name(inst.write_addr);
    const auto write_data = context->get_register_name(inst.write_data);
    const auto &memory_info = context->memories[inst.memory_block];
    out << fmt::format("{} = RAM {} {} {} {} {} {}", output, memory_info.addr_size, memory_info.word_size, read_addr,
                       write_enable, write_addr, write_data);
  }
};

// ========================================================
// class Disassembler
// ========================================================

void Disassembler::disassemble(const Instruction *instruction, const std::shared_ptr<Program> &context) {
  disassemble(instruction, context, std::cout);
}

void Disassembler::disassemble(const Instruction *instruction, const std::shared_ptr<Program> &context,
                               std::ostream &out) {
  assert(instruction != nullptr && context != nullptr);

  DisassemblerVisitor visitor(context, out);
  instruction->visit(visitor);
}

void Disassembler::disassemble(const std::shared_ptr<Program> &program) {
  disassemble(program, std::cout);
}

void Disassembler::disassemble(const std::shared_ptr<Program> &program, std::ostream &out) {
  assert(program != nullptr);

  DisassemblerVisitor visitor(program, out);

  out << "INPUT ";
  const auto inputs = program->get_inputs();
  for (auto it = inputs.begin(); it != inputs.end(); it++) {
    if (it != inputs.begin())
      out << ", ";
    out << program->get_register_name(*it);
  }
  out << "\n";

  out << "OUTPUT ";
  const auto outputs = program->get_outputs();
  for (auto it = outputs.begin(); it != outputs.end(); it++) {
    if (it != outputs.begin())
      out << ", ";
    out << program->get_register_name(*it);
  }
  out << "\n";

  out << "VAR ";
  std::uint_least32_t i = 0;
  for (auto it = program->registers.begin(); it != program->registers.end(); it++) {
    if (it != program->registers.begin())
      out << ", ";
    out << program->get_register_name({i++});
    out << ":" << it->bus_size;
  }
  out << "\n";

  out << "IN\n";
  for (const auto &instruction : program->instructions) {
    instruction->visit(visitor);
    out << "\n";
  }
}
