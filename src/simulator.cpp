#include "simulator.hpp"
#include "utilities.hpp"

Simulator::MemoryMapper::MemoryMapper(const Program::ptr &p) : current_index(0) {
  for (const auto &[var, eq] : p->get_equations()) {
    current_var = var;
    visit(eq);
  }
}

void Simulator::MemoryMapper::visit_reg_expr(const RegExpression &expr) {
  if (!reg_info.contains(expr.get_variable())) { // Can have multiple REG(o) in the program.
    reg_info.emplace(expr.get_variable(), current_index);
    current_index++;
  }
}

void Simulator::MemoryMapper::visit_ram_expr(const RamExpression &expr) {
  const size_t block_size = static_cast<size_t>(1) << expr.get_address_size();
  ram_info.emplace(std::piecewise_construct,
                   std::forward_as_tuple(current_var->get_name()),
                   std::forward_as_tuple(expr.get_write_enable(),
                                         expr.get_write_address(),
                                         expr.get_write_data(),
                                         current_index,
                                         block_size));
  current_index += block_size;
}

void Simulator::MemoryMapper::visit_rom_expr(const RomExpression &expr) {
  const size_t block_size = static_cast<size_t>(1) << expr.get_address_size();
  rom_info.emplace(std::piecewise_construct,
                   std::forward_as_tuple(current_var->get_name()),
                   std::forward_as_tuple(current_index, block_size));
  current_index += block_size;
}

Simulator::ExpressionEvaluator::ExpressionEvaluator(const Simulator::var_env &env,
                                                    const std::vector<value_t> &mem,
                                                    const Simulator::MemoryMapper &mem_map)
    : env(env), current_value(0), current_value_size(0), mem(mem), mem_map(mem_map) {}

constexpr value_t one = 1;

value_t Simulator::ExpressionEvaluator::eval(const Variable::ptr &var, const Expression::ptr &expr) noexcept {
  const value_t mask = (one << expr->get_bus_size()) - 1;
  current_value = 0;
  current_var = var;
  visit(expr);
  return current_value & mask;
}

void Simulator::ExpressionEvaluator::visit_constant(const Constant::ptr &cst) {
  current_value = cst->get_value();
  current_value_size = cst->get_bus_size();
}

void Simulator::ExpressionEvaluator::visit_variable(const Variable::ptr &var) {
  current_value = env.at(var);
  current_value_size = var->get_bus_size();
}

void Simulator::ExpressionEvaluator::visit_arg_expr(const ArgExpression &expr) {
  visit(expr.get_argument());
  // this->value contains the correct value and so this->current_value_size
}

void Simulator::ExpressionEvaluator::visit_reg_expr(const RegExpression &expr) {
  current_value = mem[mem_map.reg_info.at(expr.get_variable())];
  current_value_size = expr.get_bus_size();
}

void Simulator::ExpressionEvaluator::visit_not_expr(const NotExpression &expr) {
  // NOT o
  visit(expr.get_argument());
  // this->value contain the value of o
  current_value = ~current_value; // Not operator applied here.
  // No need to change current_value_bus_size
}

void Simulator::ExpressionEvaluator::visit_binop_expr(const BinOpExpression &expr) {
  visit(expr.get_lhs_argument());
  value_t lhs_value = current_value;
  visit(expr.get_rhs_argument());
  switch (expr.get_binop_kind()) {
  case BinOpExpression::BinOpKind::OR: {
    current_value = lhs_value | current_value;
    return;
  }
  case BinOpExpression::BinOpKind::XOR: {
    current_value = lhs_value ^ current_value;
    return;
  }
  case BinOpExpression::BinOpKind::AND: {
    current_value = lhs_value & current_value;
    return;
  }
  case BinOpExpression::BinOpKind::NAND: {
    current_value = ~(lhs_value & current_value);
    return;
  }
  }
  // No need to change current_value_bus_size too
}

void Simulator::ExpressionEvaluator::visit_mux_expr(const MuxExpression &expr) {
  visit(expr.get_choice_argument());
  // current_value_size is one here.
  if (current_value) {
    visit(expr.get_true_argument());
  } else {
    visit(expr.get_false_argument());
  }

  current_value_size = expr.get_bus_size();
}

void Simulator::ExpressionEvaluator::visit_rom_expr(const RomExpression &expr) {
  visit(expr.get_read_address());
  size_t mem_addr = mem_map.rom_info.at(current_var->get_name()).block_index + current_value;
  current_value = mem[mem_addr];
  //  TODO : Test this
  current_value_size = expr.get_bus_size();
}

void Simulator::ExpressionEvaluator::visit_ram_expr(const RamExpression &expr) {
  visit(expr.get_read_address());
  size_t mem_addr = mem_map.ram_info.at(current_var->get_name()).block_index + current_value;
  current_value = mem[mem_addr];
  //  TODO : Test this
  current_value_size = expr.get_bus_size();
}

void Simulator::ExpressionEvaluator::visit_concat_expr(const ConcatExpression &expr) {
  visit(expr.get_beginning_part());
  value_t beg_value = current_value;
  visit(expr.get_last_part());
  current_value = beg_value << current_value_size | current_value;
}

void Simulator::ExpressionEvaluator::visit_slice_expr(const SliceExpression &expr) {
  visit(expr.get_argument());
  const value_t value_shifted = current_value >> (current_value_size - expr.get_end_index() - 1);
  const value_t mask = (one << (expr.get_end_index() - expr.get_begin_index() + 1)) - 1;
  current_value = value_shifted & mask;
}

void Simulator::ExpressionEvaluator::visit_select_expr(const SelectExpression &expr) {
  visit(expr.get_argument());
  current_value = (current_value >> (current_value_size - expr.get_index() - 1)) & one;
  current_value_size = 1;
}

Simulator::Simulator(const ReportContext &ctx, const InputManager &in_manager, const Program::ptr &p)
    : prog(p), ctx(ctx), dep_list(Scheduler::schedule(ctx, p)), mem_map(p), memory(mem_map.current_index, 0),
      env(), expr_eval(env, memory, mem_map) {

  // Initialing env variables. (All result of equation are set to 0)
  for (const auto &[var, _] : p->get_equations()) {
    env[var] = 0;
  }

  // We list every memory block to fill.
  std::unordered_set<ident_t> uninitialised_blocks;
  for (const auto &[chunk, _] : mem_map.ram_info) {
    uninitialised_blocks.insert(chunk);
  }

  for (const auto &[chunk, _] : mem_map.rom_info) {
    uninitialised_blocks.insert(chunk);
  }

  // Iterator over each memory blocs given as input.
  for (const auto &[chunk_name, memory_chunks] : in_manager.memory_blocks) {
    size_t block_size;
    std::ptrdiff_t block_index;

    if (mem_map.ram_info.contains(chunk_name)) {
      block_size = mem_map.ram_info.at(chunk_name).block_size;
      block_index = mem_map.ram_info.at(chunk_name).block_index;
    } else if (mem_map.rom_info.contains(chunk_name)) {
      block_size = mem_map.rom_info.at(chunk_name).block_size;
      block_index = mem_map.rom_info.at(chunk_name).block_index;
    } else {
      ctx.report(ReportSeverity::WARNING)
          .with_message("The memory chunk {} given as input is unused.", chunk_name)
          .with_code(50)
          .build()
          .print();
      continue;
    }

    if (block_size != memory_chunks.size()) {
      ctx.report(ReportSeverity::ERROR)
          .with_message("Expected memory chunk size of {} for variable {}. Given chunk size is {}.",
                        block_size, chunk_name, memory_chunks.size())
          .with_code(51)
          .build()
          .exit();
    }

    std::copy(memory_chunks.begin(), memory_chunks.end(), memory.begin() + block_index);
    uninitialised_blocks.erase(chunk_name);
  }

  for (const ident_t &chunk_name : uninitialised_blocks) {
    if (mem_map.rom_info.contains(chunk_name)) {
      // The ROM memory block of chunk_name is not initialised -> Error.
      ctx.report(ReportSeverity::ERROR)
          .with_message("The ROM memory chunk {} is not initialised.", chunk_name)
          .with_code(52)
          .build()
          .print();

    } else {
      // It's a RAM block -> warning.
      ctx.report(ReportSeverity::WARNING)
          .with_message("The RAM memory chunk {} is not initialised.", chunk_name)
          .with_code(52)
          .build()
          .print();

    }
  }
}

void Simulator::print_env() const noexcept {
  std::cout << "Environment:\n";
  for (const auto &[var, val] : env) {
    std::cout << var->get_name() << " = " << get_output_value(var) << "\n";
  }
  std::cout << "\n" << std::flush;
}

void print_memory(const std::vector<value_t> &mem) {
  std::cout << "Memory:\n";
  if (mem.empty()) {
    std::cout << "  empty";
  } else {
    for (size_t i = 0; i < mem.size(); ++i) {

      std::cout << "memory[" << i << "] -> " << fmt::format("{:064b}", mem[i]) << "\n";
    }
  }

  std::cout << "\n" << std::flush;
}

void Simulator::cycle() {
  // Set all Input value
  for (const Variable::ptr &in_var : prog->get_inputs()) {
    env[in_var] = Parser::get_input_value(in_var);
  }

//  print_env(env);
//  print_memory(memory);

  // Iterate over equations
  for (const Variable::ptr &var : dep_list) {
    env[var] = expr_eval.eval(var, prog->get_equations().at(var));
  }

  // Save Registers to memory
  for (const auto &[var, reg_offset] : mem_map.reg_info) {
    memory[reg_offset] = env.at(var);
  }

  // Perform RAM writes
  for (const auto &[var, ram_info] : mem_map.ram_info) {
    if (eval_arg(ram_info.write_enable)) {
      value_t data = eval_arg(ram_info.data);
      size_t offset = ram_info.block_index + eval_arg(ram_info.write_addr);
      memory[offset] = data;
    }
  }

//  print_env(env);
//  print_memory(memory);
}

value_t Simulator::eval_arg(const Argument::ptr &arg) const noexcept {
  switch (arg->get_kind()) {
  case Argument::Kind::CONSTANT: {
    return std::static_pointer_cast<const Constant>(arg)->get_value();
  }
  case Argument::Kind::VARIABLE: {
    return env.at(std::static_pointer_cast<const Variable>(arg));
  }
  default: {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Unable to determine Argument Kind.")
        .with_code(92)
        .build()
        .exit();
  }
  }
}

std::string_view Simulator::get_output_value(const Variable::ptr &var) const noexcept {
  return Utilities::value_to_str(env.at(var), var->get_bus_size());
}

void Simulator::print_outputs(std::ostream &out) const {
  for (const Variable::ptr &out_var : prog->get_outputs()) {
    out << "=> " << out_var->get_name() << " = " << get_output_value(out_var) << "\n";
  }
}

