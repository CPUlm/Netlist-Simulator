#include "simulator.hpp"

#include <cassert>
#include <fmt/format.h>

/* --------------------------------------------------------
 * class Simulator
 */

// ------------------------
// The registers API

reg_value_t Simulator::get_register(reg_t reg) const {
  assert(is_valid_register(reg));

  const auto mask = (1 << m_d.program.registers[reg.index].bus_size) - 1;
  return m_d.registers_value[reg.index] & mask;
}

void Simulator::set_register(reg_t reg, reg_value_t value) {
  assert(is_valid_register(reg));
  m_d.registers_value[reg.index] = value;
}

void Simulator::print_registers(std::size_t registers_start, std::size_t registers_end) {
  // Adjust the given register range to be a valid range.
  registers_end = std::min(registers_end, m_d.program.registers.size() - 1);

  fmt::println("Registers:");

  if (registers_start != 0)
    fmt::println("  - ...");

  // Prints the registers value (in binary of course).
  for (std::size_t i = registers_start; i <= registers_end; ++i) {
    const auto &reg_info = m_d.program.registers[i];
    const auto mask = (1 << reg_info.bus_size) - 1;
    const auto current_value = m_d.registers_value[i] & mask;
    const auto previous_value = m_d.previous_registers_value[i] & mask;
    // Prints something like `  - regs[0] = 0b00101 (prev 0b00100)`
    // The `+ 2` in the code below is because we must consider that `0b` is also printed.
    fmt::println("  - %{} = {:#0{}b} (prev {:#0{}b})", i, current_value, reg_info.bus_size + 2, previous_value,
                 reg_info.bus_size + 2);
  }

  if (registers_end >= m_d.program.registers.size())
    fmt::println("  - ...");
}

void Simulator::print_ram(std::size_t region_start, std::size_t region_end) {
  // TODO: implement RAM printing
}

void Simulator::execute(size_t cycles) {
  while (cycles-- > 0) {
    while (!at_end())
      step();

    m_d.pc = 0;
  }
}

void Simulator::step() {
  m_d.program.instructions[m_d.pc]->visit(m_d);
  ++m_d.pc;
}

Simulator::Detail::Detail(const Program &program) : program(program) {
  registers_value = std::make_unique<reg_value_t[]>(program.registers.size());
  previous_registers_value = std::make_unique<reg_value_t[]>(program.registers.size());
}

void Simulator::Detail::visit_const(const ConstInstruction &inst) {
  registers_value[inst.output.index] = inst.value;
}

void Simulator::Detail::visit_not(const NotInstruction &inst) {
  // We don't want a logical not but a bitwise not.
  registers_value[inst.output.index] = ~(registers_value[inst.input.index]);
}

void Simulator::Detail::visit_and(const AndInstruction &inst) {
  const auto lhs = registers_value[inst.lhs.index];
  const auto rhs = registers_value[inst.rhs.index];
  // We don't want a logical and, but a bitwise and.
  registers_value[inst.output.index] = lhs & rhs;
}

void Simulator::Detail::visit_nand(const NandInstruction &inst) {
  const auto lhs = registers_value[inst.lhs.index];
  const auto rhs = registers_value[inst.rhs.index];
  registers_value[inst.output.index] = lhs | rhs;
}

void Simulator::Detail::visit_or(const OrInstruction &inst) {
  const auto lhs = registers_value[inst.lhs.index];
  const auto rhs = registers_value[inst.rhs.index];
  // We let the C++ compiler select instruction to implement NAND (there is no NAND operator in C++).
  registers_value[inst.output.index] = ~(lhs & rhs);
}

void Simulator::Detail::visit_nor(const NorInstruction &inst) {
  const auto lhs = registers_value[inst.lhs.index];
  const auto rhs = registers_value[inst.rhs.index];
  // We let the C++ compiler select instruction to implement NOR (there is no NOR operator in C++).
  registers_value[inst.output.index] = ~(lhs | rhs);
}

void Simulator::Detail::visit_xor(const XorInstruction &inst) {
  const auto lhs = registers_value[inst.lhs.index];
  const auto rhs = registers_value[inst.rhs.index];
  registers_value[inst.output.index] = lhs ^ rhs;
}

void Simulator::Detail::visit_reg(const RegInstruction &inst) {
  const auto previous_value = previous_registers_value[inst.input.index];
  registers_value[inst.output.index] = previous_value;
}

void Simulator::Detail::visit_slice(const SliceInstruction &inst) {
  // The `+ 1` is because both end and first are inclusives.
  const auto bit_width = inst.end - inst.start + 1;

  const auto value = registers_value[inst.input.index];
  // Mask is a binary integer whose least significant bit_width bits are set to 1.
  const auto mask = (1 << (bit_width)) - 1;
  registers_value[inst.output.index] = (value >> inst.start) & mask;
}

void Simulator::Detail::visit_select(const SelectInstruction &inst) {
  const auto value = registers_value[inst.input.index];
  registers_value[inst.output.index] = (value >> inst.i) & 0b1;
}
