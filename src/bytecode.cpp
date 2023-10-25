#include "bytecode.hpp"

#include <algorithm>
#include <cassert>

#include <fmt/format.h>

/* --------------------------------------------------------
 * class ByteCodeWriter
 */

RegIndex ByteCodeWriter::register_reg(uint8_t bit_width) {
  const RegIndex reg = m_registers.size();
  m_registers.push_back({bit_width});
  return reg;
}

void ByteCodeWriter::write_nop() {
  m_bytecode.push_back(static_cast<std::uint32_t>(OpCode::NOP));
}

void ByteCodeWriter::write_const(const ConstInstruction &data) {
  assert(is_valid_register(data.output));
  write_instruction(OpCode::CONST, data);
}

void ByteCodeWriter::write_not(const NotInstruction &data) {
  assert(is_valid_register(data.output));
  assert(is_valid_register(data.input));
  assert(m_registers[data.output].bit_width == m_registers[data.input].bit_width);
  write_instruction(OpCode::NOT, data);
}

void ByteCodeWriter::write_binary_inst(OpCode binary_opcode, const BinaryInstruction &data) {
  assert(is_valid_register(data.output));
  assert(is_valid_register(data.input_lhs));
  assert(is_valid_register(data.input_rhs));
  assert(m_registers[data.output].bit_width == m_registers[data.input_lhs].bit_width);
  assert(m_registers[data.output].bit_width == m_registers[data.input_rhs].bit_width);
  write_instruction(binary_opcode, data);
}

void ByteCodeWriter::write_and(const BinaryInstruction &data) {
  write_binary_inst(OpCode::AND, data);
}

void ByteCodeWriter::write_or(const BinaryInstruction &data) {
  write_binary_inst(OpCode::OR, data);
}

void ByteCodeWriter::write_nand(const BinaryInstruction &data) {
  write_binary_inst(OpCode::NAND, data);
}

void ByteCodeWriter::write_nor(const BinaryInstruction &data) {
  write_binary_inst(OpCode::NOR, data);
}

void ByteCodeWriter::write_xor(const BinaryInstruction &data) {
  write_binary_inst(OpCode::XOR, data);
}

void ByteCodeWriter::write_reg(const RegInstruction &data) {
  assert(is_valid_register(data.output));
  assert(is_valid_register(data.input));
  assert(m_registers[data.output].bit_width == m_registers[data.input].bit_width);
  write_instruction(OpCode::REG, data);
}

void ByteCodeWriter::write_slice(const SliceInstruction &data) {
  assert(is_valid_register(data.output));
  assert(is_valid_register(data.input));
  assert(data.end > data.first);
  assert(data.first < m_registers[data.input].bit_width);
  assert(data.end < m_registers[data.input].bit_width);
  assert(m_registers[data.output].bit_width == (data.end - data.first + 1));
  write_instruction(OpCode::SLICE, data);
}

void ByteCodeWriter::write_select(const SelectInstruction &data) {
  assert(is_valid_register(data.output));
  assert(is_valid_register(data.input));
  assert(data.i < m_registers[data.input].bit_width);
  assert(m_registers[data.output].bit_width == 1);
  write_instruction(OpCode::SELECT, data);
}

ByteCode ByteCodeWriter::finish() {
  return ByteCode{std::move(m_registers), std::move(m_bytecode)};
}

/* --------------------------------------------------------
 * class Disassembler
 */

void Disassembler::disassemble(const ByteCode &bytecode, std::FILE *stream) {
  fmt::println(stream, "# Registers:");
  for (size_t i = 0; i < bytecode.registers.size(); ++i) {
    const auto &reg_info = bytecode.registers[i];
    fmt::println(stream, "# - r{}: {}", i, reg_info.bit_width);
  }

  Detail printer(bytecode);
  printer.stream = stream;
  printer.read_all();
}

void Disassembler::Detail::handle_nop() const {
  fmt::println(stream, "NOP");
}

void Disassembler::Detail::handle_break() const {
  fmt::println(stream, "BREAK");
}

void Disassembler::Detail::handle_const(const ConstInstruction &inst) const {
  fmt::println(stream, "CONST r{}, {}", inst.output, inst.value);
}

void Disassembler::Detail::handle_not(const NotInstruction &inst) const {
  fmt::println(stream, "NOT r{}, r{}", inst.output, inst.input);
}

void Disassembler::Detail::handle_binary_inst(const char *name, const BinaryInstruction &inst) const {
  fmt::println(stream, "{} r{}, r{}, r{}", name, inst.output, inst.input_lhs, inst.input_rhs);
}

void Disassembler::Detail::handle_and(const BinaryInstruction &inst) const {
  handle_binary_inst("AND", inst);
}

void Disassembler::Detail::handle_or(const BinaryInstruction &inst) const {
  handle_binary_inst("OR", inst);
}

void Disassembler::Detail::handle_nand(const BinaryInstruction &inst) const {
  handle_binary_inst("NAND", inst);
}

void Disassembler::Detail::handle_nor(const BinaryInstruction &inst) const {
  handle_binary_inst("NOR", inst);
}

void Disassembler::Detail::handle_xor(const BinaryInstruction &inst) const {
  handle_binary_inst("XOR", inst);
}

void Disassembler::Detail::handle_reg(const RegInstruction &inst) const {
  fmt::println(stream, "REG r{}, r{}", inst.output, inst.input);
}

void Disassembler::Detail::handle_slice(const SliceInstruction &inst) const {
  fmt::println(stream, "SLICE r{}, r{}, {}, {}", inst.output, inst.input, inst.first, inst.end);
}

void Disassembler::Detail::handle_select(const SelectInstruction &inst) const {
  fmt::println(stream, "SELECT r{}, r{}, {}", inst.output, inst.input, inst.i);
}
