#include "program.hpp"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <ostream>

/* --------------------------------------------------------
 * class ProgramBuilder
 */

reg_t ProgramBuilder::add_register(bus_size_t bus_size, const std::string &name) {
  assert(m_program.registers.size() < UINT_LEAST32_MAX && "too many registers allocated");

  reg_t reg = {static_cast<std::uint_least32_t>(m_program.registers.size())};
  RegisterInfo &info = m_program.registers.emplace_back();
  info.reg = reg;
  info.name = name;
  info.bus_size = bus_size;
  return reg;
}

ConstInstruction &ProgramBuilder::add_const(reg_t output, reg_value_t value) {
  assert(check_reg(output));

  auto *inst = new ConstInstruction();
  inst->output = output;
  inst->value = value;
  m_program.instructions.push_back(inst);
  return *inst;
}

NotInstruction &ProgramBuilder::add_not(reg_t output, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new NotInstruction();
  inst->output = output;
  inst->input = input;
  m_program.instructions.push_back(inst);
  return *inst;
}

AndInstruction &ProgramBuilder::add_and(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new AndInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

NandInstruction &ProgramBuilder::add_nand(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new NandInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

OrInstruction &ProgramBuilder::add_or(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new OrInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

NorInstruction &ProgramBuilder::add_nor(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new NorInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

XorInstruction &ProgramBuilder::add_xor(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new XorInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

XnorInstruction &ProgramBuilder::add_xnor(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new XnorInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

ConcatInstruction &ProgramBuilder::add_concat(reg_t output, reg_t lhs, reg_t rhs) {
  assert(check_reg(output) && check_reg(lhs) && check_reg(rhs));

  auto *inst = new ConcatInstruction();
  inst->output = output;
  inst->lhs = lhs;
  inst->rhs = rhs;
  m_program.instructions.push_back(inst);
  return *inst;
}

RegInstruction &ProgramBuilder::add_reg(reg_t output, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new RegInstruction();
  inst->output = output;
  inst->input = input;
  m_program.instructions.push_back(inst);
  return *inst;
}

MuxInstruction &ProgramBuilder::add_mux(reg_t output, reg_t choice, reg_t first, reg_t second) {
  assert(check_reg(output) && check_reg(choice) && check_reg(first) && check_reg(second));

  auto *inst = new MuxInstruction();
  inst->output = output;
  inst->choice = choice;
  inst->first = first;
  inst->second = second;
  m_program.instructions.push_back(inst);
  return *inst;
}

SelectInstruction &ProgramBuilder::add_select(reg_t output, bus_size_t i, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new SelectInstruction();
  inst->output = output;
  inst->i = i;
  inst->input = input;
  m_program.instructions.push_back(inst);
  return *inst;
}

SliceInstruction &ProgramBuilder::add_slice(reg_t output, bus_size_t start, bus_size_t end, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new SliceInstruction();
  inst->output = output;
  inst->start = start;
  inst->end = end;
  inst->input = input;
  m_program.instructions.push_back(inst);
  return *inst;
}

[[nodiscard]] Program ProgramBuilder::build() {
  return std::move(m_program);
}

[[nodiscard]] bool ProgramBuilder::check_reg(reg_t reg) const {
  return reg.index < m_program.registers.size();
}

/* --------------------------------------------------------
 * class Disassembler
 */

void Disassembler::disassemble(const Instruction &instruction) {
  disassemble(instruction, std::cout);
}

void Disassembler::disassemble(const Instruction &instruction, std::ostream &out) {
  Printer printer(out);
  instruction.visit(printer);
}

void Disassembler::disassemble(const Program &program) {
  disassemble(program, std::cout);
}

void Disassembler::disassemble(const Program &program, std::ostream &out) {
  Printer printer(out);
  for (const auto &instruction : program.instructions) {
    instruction->visit(printer);
    out << "\n";
  }
}

void Disassembler::Printer::print_reg(reg_t reg) {
  out << "%" << reg.index;
}

void Disassembler::Printer::print_inst_label(const char *opcode, reg_t output) {
  print_reg(output);
  out << " = " << opcode << " ";
}

void Disassembler::Printer::print_binary_inst(const char *opcode, const BinaryInstruction &inst) {
  print_inst_label(opcode, inst.output);
  print_reg(inst.lhs);
  out << " ";
  print_reg(inst.rhs);
}

void Disassembler::Printer::visit_const(const ConstInstruction &inst) {
  print_reg(inst.output);
  out << " = ";
  // TODO(hgruniaux): better output for constants (maybe outputting in binary
  //  and considering the bus size).
  out << inst.value;
}

void Disassembler::Printer::visit_not(const NotInstruction &inst) {
  print_inst_label("NOT", inst.output);
  print_reg(inst.input);
}

void Disassembler::Printer::visit_reg(const RegInstruction &inst) {
  print_inst_label("REG", inst.output);
  print_reg(inst.input);
}

void Disassembler::Printer::visit_mux(const MuxInstruction &inst) {
  print_inst_label("MUX", inst.output);
  print_reg(inst.choice);
  out << " ";
  print_reg(inst.first);
  out << " ";
  print_reg(inst.second);
}

void Disassembler::Printer::visit_concat(const ConcatInstruction &inst) {
  print_inst_label("CONCAT", inst.output);
  print_reg(inst.lhs);
  out << " ";
  print_reg(inst.rhs);
}

void Disassembler::Printer::visit_and(const AndInstruction &inst) {
  print_binary_inst("AND", inst);
}

void Disassembler::Printer::visit_nand(const NandInstruction &inst) {
  print_binary_inst("NAND", inst);
}

void Disassembler::Printer::visit_or(const OrInstruction &inst) {
  print_binary_inst("OR", inst);
}

void Disassembler::Printer::visit_nor(const NorInstruction &inst) {
  print_binary_inst("NOR", inst);
}

void Disassembler::Printer::visit_xor(const XorInstruction &inst) {
  print_binary_inst("XOR", inst);
}

void Disassembler::Printer::visit_xnor(const XnorInstruction &inst) {
  print_binary_inst("XNOR", inst);
}

void Disassembler::Printer::visit_select(const SelectInstruction &inst) {
  print_inst_label("SELECT", inst.output);
  print_reg(inst.input);
  // TODO: print SELECT instruction
}

void Disassembler::Printer::visit_slice(const SliceInstruction &inst) {
  print_inst_label("SLICE", inst.output);
  print_reg(inst.input);
  // TODO: print SLICE instruction
}

void Disassembler::Printer::visit_rom(const RomInstruction &inst) {
  print_inst_label("ROM", inst.output);
  // TODO: print ROM instruction
}

void Disassembler::Printer::visit_ram(const RamInstruction &inst) {
  print_inst_label("RAM", inst.output);
  // TODO: print RAM instruction
}
