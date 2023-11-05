#include "program.hpp"

#include <algorithm>
#include <cassert>
#include <fmt/format.h>
#include <iostream>
#include <ostream>

// ========================================================
// struct Program
// ========================================================

bool Program::has_inputs() const {
  return std::ranges::any_of(registers, [](const auto &register_info) { return register_info.flags & RIF_INPUT; });
}

std::vector<reg_t> Program::get_inputs() const {
  std::vector<reg_t> inputs;
  for (reg_index_t i = 0; i < registers.size(); ++i) {
    if (registers[i].flags & RIF_INPUT)
      inputs.push_back({i});
  }
  return inputs;
}

bool Program::has_outputs() const {
  return std::ranges::any_of(registers, [](const auto &register_info) { return register_info.flags & RIF_OUTPUT; });
}

std::vector<reg_t> Program::get_outputs() const {
  std::vector<reg_t> outputs;
  for (reg_index_t i = 0; i < registers.size(); ++i) {
    if (registers[i].flags & RIF_OUTPUT)
      outputs.push_back({i});
  }
  return outputs;
}

std::string Program::get_register_name(reg_t reg) const {
  assert(reg.index < registers.size());

  const auto &register_info = registers[reg.index];
  if (register_info.name.empty())
    return fmt::format("__r{}", reg.index);
  else
    return register_info.name;
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

bus_size_t ProgramBuilder::get_register_bus_size(reg_t reg) const {
  assert(check_reg(reg));
  return m_program->registers[reg.index].bus_size;
}

ConstInstruction &ProgramBuilder::add_const(reg_t output, reg_value_t value) {
  assert(check_reg(output));

  auto *inst = new ConstInstruction();
  inst->output = output;
  inst->value = value;
  m_program->instructions.push_back(inst);
  return *inst;
}

LoadInstruction &ProgramBuilder::add_load(reg_t output, reg_t input) {
  assert(check_reg(output) && check_reg(input));

  auto *inst = new LoadInstruction();
  inst->output = output;
  inst->input = input;
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
  inst->offset = m_program->registers[lhs.index].bus_size;
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

RomInstruction &ProgramBuilder::add_rom(reg_t output, bus_size_t addr_size, bus_size_t word_size, reg_t read_addr) {
  assert(check_reg(output) && check_reg(read_addr));

  auto *inst = new RomInstruction();
  inst->output = output;
  inst->memory_block = m_program->memories.size();
  inst->read_addr = read_addr;
  m_program->instructions.push_back(inst);

  auto &memory_info = m_program->memories.emplace_back();
  memory_info.parent = inst;
  memory_info.addr_size = addr_size;
  memory_info.word_size = word_size;

  return *inst;
}

RamInstruction &ProgramBuilder::add_ram(reg_t output, bus_size_t addr_size, bus_size_t word_size, reg_t read_addr,
                                        reg_t write_enable, reg_t write_addr, reg_t write_data) {
  assert(check_reg(output) && check_reg(read_addr) && check_reg(write_enable) && check_reg(write_addr) &&
         check_reg(write_data));

  auto *inst = new RamInstruction();
  inst->output = output;
  inst->memory_block = m_program->memories.size();
  inst->read_addr = read_addr;
  inst->write_enable = write_enable;
  inst->write_addr = write_addr;
  inst->write_data = write_data;
  m_program->instructions.push_back(inst);

  auto &memory_info = m_program->memories.emplace_back();
  memory_info.parent = inst;
  memory_info.addr_size = addr_size;
  memory_info.word_size = word_size;

  return *inst;
}

std::shared_ptr<Program> ProgramBuilder::build() {
  return std::move(m_program);
}

bool ProgramBuilder::check_reg(reg_t reg) const {
  return reg.index < m_program->registers.size();
}
