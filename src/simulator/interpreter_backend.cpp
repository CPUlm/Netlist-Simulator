#include "interpreter_backend.hpp"

#include <cstring>

// ========================================================
// class InterpreterBackend::Detail
// ========================================================

struct InterpreterBackend::Detail final : ConstInstructionVisitor {
  std::shared_ptr<Program> program;
  size_t pc = 0; // the program counter
  std::vector<reg_value_t> registers_value;
  std::vector<reg_value_t> saved_registers_value;
  std::vector<std::unique_ptr<reg_value_t[]>> memory_blocks;
  std::vector<std::unique_ptr<reg_value_t[]>> saved_memory_blocks;

  /// Checks if the given register is valid.
  [[nodiscard]] bool check_reg(reg_t reg) const { return reg.index < program->registers.size(); }

  /// Returns true if the end of the program was reached.
  [[nodiscard]] bool at_end() const { return pc >= program->instructions.size(); }

  void prepare(const std::shared_ptr<Program> &p) {
    program = p;
    pc = 0;
    registers_value.resize(program->registers.size());
    saved_registers_value.resize(program->registers.size());
    // Zero-initialize the registers just to be sure.
    std::memset(registers_value.data(), 0, sizeof(reg_value_t) * registers_value.size());
    std::memset(saved_registers_value.data(), 0, sizeof(reg_value_t) * saved_registers_value.size());

    memory_blocks.resize(program->memories.size());
    saved_memory_blocks.resize(program->memories.size());
    for (uint_least32_t i = 0; i < program->memories.size(); ++i) {
      const auto &memory_info = program->memories[i];
      memory_blocks[i] = std::make_unique<reg_value_t[]>(memory_info.get_size());
      saved_memory_blocks[i] = std::make_unique<reg_value_t[]>(memory_info.get_size());
    }
  }

  void step() {
    program->instructions[pc]->visit(*this);
    ++pc;
  }

  void start_cycle() { pc = 0; }

  void end_cycle() {
    // Save registers.
    std::memcpy(saved_registers_value.data(), registers_value.data(), sizeof(reg_value_t) * registers_value.size());

    // Save memory blocks.
    for (uint_least32_t i = 0; i < program->memories.size(); ++i) {
      const auto &memory_info = program->memories[i];
      std::memcpy(saved_memory_blocks[i].get(), memory_blocks[i].get(), sizeof(reg_value_t) * memory_info.get_size());
    }
  }

  void cycle() {
    start_cycle();

    while (!at_end())
      step();

    end_cycle();
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
    registers_value[inst.output.index] = lhs | (rhs << inst.offset);
  }

  void visit_reg(const RegInstruction &inst) override {
    const auto previous_value = saved_registers_value[inst.input.index];
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
    const reg_value_t *memory_block = saved_memory_blocks[inst.memory_block].get();
    registers_value[inst.output.index] = memory_block[read_addr];
  }

  void visit_ram(const RamInstruction &inst) override {
    const auto read_addr = registers_value[inst.read_addr.index];
    const auto write_enable = registers_value[inst.write_enable.index];
    const auto write_addr = registers_value[inst.write_addr.index];
    const auto write_data = registers_value[inst.write_data.index];

    const reg_value_t *read_memory_block = saved_memory_blocks[inst.memory_block].get();
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
  return m_d->registers_value.data();
}

bool InterpreterBackend::prepare(const std::shared_ptr<Program> &program) {
  m_d->prepare(program);
  return true;
}

void InterpreterBackend::cycle() {
  m_d->cycle();
}
