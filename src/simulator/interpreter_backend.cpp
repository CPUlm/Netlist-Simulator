#include "interpreter_backend.hpp"

#include <cassert>

// ========================================================
// class InterpreterBackend::Detail
// ========================================================

struct InterpreterBackend::Detail final : ConstInstructionVisitor {
  std::shared_ptr<Program> program;
  size_t pc = 0; // the program counter
  std::unique_ptr<reg_value_t[]> registers_value;
  std::unique_ptr<reg_value_t[]> previous_registers_value;
  std::unique_ptr<std::unique_ptr<reg_value_t[]>[]> memory_blocks;
  std::unique_ptr<std::unique_ptr<reg_value_t[]>[]> previous_memory_blocks;

  /// Checks if the given register is valid.
  [[nodiscard]] bool check_reg(reg_t reg) const { return reg.index < program->registers.size(); }

  /// Returns true if the end of the program was reached.
  [[nodiscard]] bool at_end() const { return pc >= program->instructions.size(); }

  void prepare(const std::shared_ptr<Program> &p) {
    program = p;
    pc = 0;
    registers_value = std::make_unique<reg_value_t[]>(program->registers.size());
    previous_registers_value = std::make_unique<reg_value_t[]>(program->registers.size());

    memory_blocks = std::make_unique<std::unique_ptr<reg_value_t[]>[]>(program->memories.size());
    previous_memory_blocks = std::make_unique<std::unique_ptr<reg_value_t[]>[]>(program->memories.size());
    for (uint_least32_t i = 0; i < program->memories.size(); ++i) {
      const auto &memory_info = program->memories[i];
      memory_blocks[i] = std::make_unique<reg_value_t[]>(memory_info.get_size());
      previous_memory_blocks[i] = std::make_unique<reg_value_t[]>(memory_info.get_size());
    }
  }

  void step() {
    program->instructions[pc]->visit(*this);
    ++pc;
  }

  void finish_cycle() {
    pc = 0;
    std::swap(previous_registers_value, registers_value);
    std::swap(previous_memory_blocks, memory_blocks);
  }

  void execute(size_t n = 1) {
    while (n-- > 0) {
      while (!at_end())
        step();

      finish_cycle();
    }

    std::swap(previous_registers_value, registers_value);
    std::swap(previous_memory_blocks, memory_blocks);
  }

  void visit_const(const ConstInstruction &inst) override { registers_value[inst.output.index] = inst.value; }

  void visit_load(const LoadInstruction &inst) override {
    registers_value[inst.output.index] = registers_value[inst.input.index];
  }

  void visit_not(const NotInstruction &inst) override {
    registers_value[inst.output.index] = ~(registers_value[inst.input.index]);
  }

  void visit_and(const AndInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = lhs & rhs;
  }

  void visit_nand(const NandInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = ~(lhs & rhs);
  }

  void visit_or(const OrInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = lhs | rhs;
  }

  void visit_nor(const NorInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = ~(lhs | rhs);
  }

  void visit_xor(const XorInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = lhs ^ rhs;
  }

  void visit_xnor(const XnorInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = ~(lhs ^ rhs);
  }

  void visit_concat(const ConcatInstruction &inst) override {
    const auto lhs = registers_value[inst.lhs.index];
    const auto rhs = registers_value[inst.rhs.index];
    registers_value[inst.output.index] = (lhs << inst.offset) | rhs;
  }

  void visit_reg(const RegInstruction &inst) override {
    const auto previous_value = previous_registers_value[inst.input.index];
    registers_value[inst.output.index] = previous_value;
  }

  void visit_mux(const MuxInstruction &inst) override {
    const auto choice = registers_value[inst.choice.index];
    const auto first = registers_value[inst.first.index];
    const auto second = registers_value[inst.second.index];
    if (choice == 0) {
      registers_value[inst.output.index] = first;
    } else {
      registers_value[inst.output.index] = second;
    }
  }

  void visit_slice(const SliceInstruction &inst) override {
    // The `+ 1` is because both end and first are inclusives.
    const auto bit_width = inst.end - inst.start + 1;

    const auto value = registers_value[inst.input.index];
    // Mask is a binary integer whose least significant bit_width bits are set to 1.
    const auto mask = (1 << (bit_width)) - 1;
    registers_value[inst.output.index] = (value >> inst.start) & mask;
  }

  void visit_select(const SelectInstruction &inst) override {
    const auto value = registers_value[inst.input.index];
    registers_value[inst.output.index] = (value >> inst.i) & 0b1;
  }

  void visit_rom(const RomInstruction &inst) override {
    const auto read_addr = registers_value[inst.read_addr.index];
    const reg_value_t *memory_block = previous_memory_blocks[inst.memory_block].get();
    registers_value[inst.output.index] = memory_block[read_addr];
  }

  void visit_ram(const RamInstruction &inst) override {
    const auto read_addr = registers_value[inst.read_addr.index];
    const auto write_enable = registers_value[inst.write_enable.index];
    const auto write_addr = registers_value[inst.write_addr.index];
    const auto write_data = registers_value[inst.write_data.index];

    const reg_value_t *read_memory_block = previous_memory_blocks[inst.memory_block].get();
    registers_value[inst.output.index] = read_memory_block[read_addr];

    if (write_enable) {
      reg_value_t *write_memory_block = memory_blocks[inst.memory_block].get();
      write_memory_block[write_addr] = write_data;
    }
  }
};

// ========================================================
// class InterpreterBackend
// ========================================================

InterpreterBackend::InterpreterBackend() : m_d(std::make_unique<InterpreterBackend::Detail>()) {
}

InterpreterBackend::~InterpreterBackend() = default;

// ------------------------------------------------------
// The simulator API
// ------------------------------------------------------

reg_value_t *InterpreterBackend::get_registers() {
  return m_d->registers_value.get();
}

bool InterpreterBackend::prepare(const std::shared_ptr<Program> &program) {
  m_d->prepare(program);
  return true;
}

void InterpreterBackend::simulate(size_t n) {
  assert(n > 0);
  m_d->execute(n);
}

// ------------------------------------------------------
// The debugger API
// ------------------------------------------------------

reg_value_t InterpreterBackend::get_register(reg_t reg) const {
  assert(m_d->check_reg(reg));

  const auto mask = (1 << m_d->program->registers[reg.index].bus_size) - 1;
  return m_d->registers_value[reg.index] & mask;
}

void InterpreterBackend::set_register(reg_t reg, reg_value_t value) {
  assert(m_d->check_reg(reg));
  m_d->registers_value[reg.index] = value;
}

void InterpreterBackend::step() {
  m_d->step();
}
