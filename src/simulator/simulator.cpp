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
// The simulator API
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

void Simulator::cycle() {
  m_backend->cycle();
}

void Simulator::simulate(size_t n) {
  m_backend->simulate(n);
}
