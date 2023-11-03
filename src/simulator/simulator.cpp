#include "simulator.hpp"

#include "interpreter_backend.hpp"

#include <cassert>
#include <fmt/format.h>

// ========================================================
// class Simulator
// ========================================================

Simulator::Simulator(const std::shared_ptr<Program> &program)
    : m_program(program), m_backend(std::make_unique<InterpreterBackend>()) {
  m_backend->prepare(m_program);
}

// ------------------------------------------------------
// The debugger API
// ------------------------------------------------------

reg_value_t Simulator::get_register(reg_t reg) const {
  assert(is_valid_register(reg));
  const auto mask = (1 << m_program->registers[reg.index].bus_size) - 1;
  return m_backend->get_registers()[reg.index] & mask;
}

void Simulator::set_register(reg_t reg, reg_value_t value) {
  assert(is_valid_register(reg));
  m_backend->get_registers()[reg.index] = value;
}

void Simulator::print_register(reg_t reg) {
  assert(is_valid_register(reg));

  fmt::print("Register");
  print_register_impl(reg);
  fmt::println("");
}

void Simulator::print_registers(std::uint_least32_t registers_start, std::uint_least32_t registers_end) {
  auto register_count = static_cast<std::uint_least32_t>(m_program->registers.size());

  // Adjust the given register range to be a valid range.
  registers_end = std::min(registers_end, register_count - 1);

  fmt::println("Registers:");

  if (registers_start != 0)
    fmt::println("  - ...");

  // Prints the registers value (in binary).
  for (std::uint_least32_t i = registers_start; i <= registers_end; ++i) {
    fmt::print("  - ");
    print_register_impl({i});
    fmt::println("");
  }

  if (registers_end >= register_count)
    fmt::println("  - ...");
}

void Simulator::print_inputs() {
  fmt::println("Input registers:");

  for (std::uint_least32_t i = 0; i < m_program->registers.size(); ++i) {
    if (!(m_program->registers[i].flags & RIF_INPUT))
      continue;

    fmt::print("  - ");
    print_register_impl({i});
    fmt::println("");
  }
}

void Simulator::print_outputs() {
  fmt::println("Output registers:");

  for (std::uint_least32_t i = 0; i < m_program->registers.size(); ++i) {
    if (!(m_program->registers[i].flags & RIF_OUTPUT))
      continue;

    fmt::print("  - ");
    print_register_impl({i});
    fmt::println("");
  }
}

void Simulator::print_register_impl(reg_t reg) {
  const auto &reg_info = m_program->registers[reg.index];
  const auto current_value = get_register(reg);
  // The `+ 2` in the code below is because we must consider that `0b` is also printed.
  fmt::print("%{} (aka '{}') = {:#0{}b}", reg.index, reg_info.name, current_value, reg_info.bus_size + 2);
}

// ------------------------------------------------------
// The simulator API
// ------------------------------------------------------

void Simulator::cycle() {
  m_backend->cycle();
}
