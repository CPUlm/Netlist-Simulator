#include "program.hpp"

#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <ostream>

// ========================================================
// class Disassembler
// ========================================================

void Disassembler::disassemble(const Instruction &instruction) {
  disassemble(instruction, std::cout);
}

void Disassembler::disassemble(const Instruction &instruction, std::ostream &out) {
  Detail printer(out);
  instruction.visit(printer);
}

void Disassembler::disassemble(const std::shared_ptr<Program> &program) {
  disassemble(program, std::cout);
}

void Disassembler::disassemble(const std::shared_ptr<Program> &program, std::ostream &out) {
  Detail d(out);
  d.program = program;
  for (const auto &instruction : program->instructions) {
    instruction->visit(d);
    out << "\n";
  }
}

void Disassembler::Detail::print_reg(reg_t reg) {
  if (program != nullptr) {
    const auto &reg_info = program->registers[reg.index];
    if (!reg_info.name.empty()) {
      out << reg_info.name;
      return;
    }
  }

  // Fallback to using a generic register syntax: `%{reg_index}`.
  out << "%" << reg.index;
}

void Disassembler::Detail::print_inst_label(const char *opcode, reg_t output) {
  print_reg(output);
  out << " = " << opcode << " ";
}

void Disassembler::Detail::print_binary_inst(const char *opcode, const BinaryInstruction &inst) {
  print_inst_label(opcode, inst.output);
  print_reg(inst.lhs);
  out << " ";
  print_reg(inst.rhs);
}

void Disassembler::Detail::visit_const(const ConstInstruction &inst) {
  print_reg(inst.output);
  out << " = ";
  // TODO(hgruniaux): better output for constants (maybe outputting in binary
  //  and considering the bus size).
  out << inst.value;
}

void Disassembler::Detail::visit_not(const NotInstruction &inst) {
  print_inst_label("NOT", inst.output);
  print_reg(inst.input);
}

void Disassembler::Detail::visit_reg(const RegInstruction &inst) {
  print_inst_label("REG", inst.output);
  print_reg(inst.input);
}

void Disassembler::Detail::visit_mux(const MuxInstruction &inst) {
  print_inst_label("MUX", inst.output);
  print_reg(inst.choice);
  out << " ";
  print_reg(inst.first);
  out << " ";
  print_reg(inst.second);
}

void Disassembler::Detail::visit_concat(const ConcatInstruction &inst) {
  print_inst_label("CONCAT", inst.output);
  print_reg(inst.lhs);
  out << " ";
  print_reg(inst.rhs);
}

void Disassembler::Detail::visit_and(const AndInstruction &inst) {
  print_binary_inst("AND", inst);
}

void Disassembler::Detail::visit_nand(const NandInstruction &inst) {
  print_binary_inst("NAND", inst);
}

void Disassembler::Detail::visit_or(const OrInstruction &inst) {
  print_binary_inst("OR", inst);
}

void Disassembler::Detail::visit_nor(const NorInstruction &inst) {
  print_binary_inst("NOR", inst);
}

void Disassembler::Detail::visit_xor(const XorInstruction &inst) {
  print_binary_inst("XOR", inst);
}

void Disassembler::Detail::visit_xnor(const XnorInstruction &inst) {
  print_binary_inst("XNOR", inst);
}

void Disassembler::Detail::visit_select(const SelectInstruction &inst) {
  print_inst_label("SELECT", inst.output);
  print_reg(inst.input);
  // TODO: print SELECT instruction
}

void Disassembler::Detail::visit_slice(const SliceInstruction &inst) {
  print_inst_label("SLICE", inst.output);
  print_reg(inst.input);
  // TODO: print SLICE instruction
}

void Disassembler::Detail::visit_rom(const RomInstruction &inst) {
  print_inst_label("ROM", inst.output);
  // TODO: print ROM instruction
}

void Disassembler::Detail::visit_ram(const RamInstruction &inst) {
  print_inst_label("RAM", inst.output);
  // TODO: print RAM instruction
}

// ========================================================
// class TestCodeGenerator
// ========================================================

void BuilderCodeGenerator::generate(const std::shared_ptr<Program> &program) {
  generate(program, std::cout);
}

void BuilderCodeGenerator::generate(const std::shared_ptr<Program> &program, std::ostream &out) {
  Detail d(out);
  d.prepare(program);
  for (auto *instruction : program->instructions) {
    instruction->visit(d);
    out << "\n";
  }
}

void BuilderCodeGenerator::Detail::prepare(const std::shared_ptr<Program> &p) {
  program = p;

  out << "ProgramBuilder builder;\n";

  for (std::uint_least32_t i = 0; i < program->registers.size(); ++i) {
    const auto &reg_info = program->registers[i];
    const auto reg_name = get_reg_name({i});

    out << fmt::format("const auto {} = builder.add_register({}, \"{}\", {});\n", reg_name, reg_info.bus_size,
                       reg_info.name, reg_info.flags);
  }
}

std::string BuilderCodeGenerator::Detail::get_reg_name(reg_t reg) const {
  const auto &reg_info = program->registers[reg.index];
  if (!reg_info.name.empty())
    return reg_info.name;

  return fmt::format("_reg_{}", reg.index);
}

void BuilderCodeGenerator::Detail::visit_const(const ConstInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  out << fmt::format("builder.add_const({}, {});", output, inst.value);
}

void BuilderCodeGenerator::Detail::visit_not(const NotInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto input = get_reg_name(inst.input);
  out << fmt::format("builder.add_not({}, {});", output, input);
}

void BuilderCodeGenerator::Detail::visit_reg(const RegInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto input = get_reg_name(inst.input);
  out << fmt::format("builder.add_reg({}, {});", output, input);
}

void BuilderCodeGenerator::Detail::visit_mux(const MuxInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto choice = get_reg_name(inst.choice);
  const auto first = get_reg_name(inst.first);
  const auto second = get_reg_name(inst.second);
  out << fmt::format("builder.add_mux({}, {}, {}, {});", output, choice, first, second);
}

void BuilderCodeGenerator::Detail::visit_concat(const ConcatInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_concat({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_and(const AndInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_and({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_nand(const NandInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_nand({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_or(const OrInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_or({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_nor(const NorInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_nor({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_xor(const XorInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_xor({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_xnor(const XnorInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto lhs = get_reg_name(inst.lhs);
  const auto rhs = get_reg_name(inst.rhs);
  out << fmt::format("builder.add_xnor({}, {}, {});", output, lhs, rhs);
}

void BuilderCodeGenerator::Detail::visit_select(const SelectInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto input = get_reg_name(inst.input);
  out << fmt::format("builder.add_select({}, {}, {});", output, inst.i, input);
}

void BuilderCodeGenerator::Detail::visit_slice(const SliceInstruction &inst) {
  const auto output = get_reg_name(inst.output);
  const auto input = get_reg_name(inst.input);
  out << fmt::format("builder.add_select({}, {}, {}, {});", output, inst.start, inst.end, input);
}

void BuilderCodeGenerator::Detail::visit_rom(const RomInstruction &inst) {
  // TODO
}

void BuilderCodeGenerator::Detail::visit_ram(const RamInstruction &inst) {
  // TODO
}

// ========================================================
// class ProgramBuilder
// ========================================================

reg_t ProgramBuilder::add_register(bus_size_t bus_size, const std::string &name, unsigned flags) {
  assert(m_program->registers.size() < UINT_LEAST32_MAX && "too many registers allocated");

  reg_t reg = {static_cast<std::uint_least32_t>(m_program->registers.size())};
  RegisterInfo &info = m_program->registers.emplace_back();
  info.name = name;
  info.bus_size = bus_size;
  info.flags = flags;
  return reg;
}

ConstInstruction &ProgramBuilder::add_const(reg_t output, reg_value_t value) {
  assert(check_reg(output));

  auto *inst = new ConstInstruction();
  inst->output = output;
  inst->value = value;
  m_program->instructions.push_back(inst);
  return *inst;
}

NotInstruction &ProgramBuilder::add_not(reg_t output, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new NotInstruction();
  inst->output = output;
  inst->input = input;
  m_program->instructions.push_back(inst);
  return *inst;
}

AndInstruction &ProgramBuilder::add_and(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new AndInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program->instructions.push_back(inst);
  return *inst;
}

NandInstruction &ProgramBuilder::add_nand(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new NandInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program->instructions.push_back(inst);
  return *inst;
}

OrInstruction &ProgramBuilder::add_or(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new OrInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program->instructions.push_back(inst);
  return *inst;
}

NorInstruction &ProgramBuilder::add_nor(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new NorInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program->instructions.push_back(inst);
  return *inst;
}

XorInstruction &ProgramBuilder::add_xor(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new XorInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program->instructions.push_back(inst);
  return *inst;
}

XnorInstruction &ProgramBuilder::add_xnor(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new XnorInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program->instructions.push_back(inst);
  return *inst;
}

ConcatInstruction &ProgramBuilder::add_concat(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new ConcatInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  inst->offset = m_program->registers[rhs.index].bus_size;
  m_program->instructions.push_back(inst);
  return *inst;
}

RegInstruction &ProgramBuilder::add_reg(reg_t output, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new RegInstruction();
  inst->output = output;
  inst->input = input;
  m_program->instructions.push_back(inst);
  return *inst;
}

MuxInstruction &ProgramBuilder::add_mux(reg_t output, reg_t choice, reg_t first, reg_t second) {
  assert(check_reg(output) && check_reg(choice) && check_reg(first) && check_reg(second));

  auto *inst = new MuxInstruction();
  inst->output = output;
  inst->choice = choice;
  inst->first = first;
  inst->second = second;
  m_program->instructions.push_back(inst);
  return *inst;
}

SelectInstruction &ProgramBuilder::add_select(reg_t output, bus_size_t i, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new SelectInstruction();
  inst->output = output;
  inst->i = i;
  inst->input = input;
  m_program->instructions.push_back(inst);
  return *inst;
}

SliceInstruction &ProgramBuilder::add_slice(reg_t output, bus_size_t start, bus_size_t end, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new SliceInstruction();
  inst->output = output;
  inst->start = start;
  inst->end = end;
  inst->input = input;
  m_program->instructions.push_back(inst);
  return *inst;
}

[[nodiscard]] std::shared_ptr<Program> ProgramBuilder::build() {
  return std::move(m_program);
}

[[nodiscard]] bool ProgramBuilder::check_reg(reg_t reg) const {
  return reg.index < m_program->registers.size();
}
