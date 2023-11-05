#ifndef NETLIST_SRC_SIMULATOR_HPP
#define NETLIST_SRC_SIMULATOR_HPP

#include <variant>

#include "program.hpp"
#include "scheduler.hpp"
#include "input_manager.hpp"

class Simulator {
public:
  explicit Simulator(const ReportContext &ctx, const InputManager &in_manager, const Program::ptr &p);

  void cycle();

  [[nodiscard]] value_t read_value(const Variable::ptr &var) const {
    return env.at(var);
  }

private:
  using var_env = std::unordered_map<Variable::ptr, value_t>;

  class MemoryMapper : public Visitor<MemoryMapper> {
  public:
    struct RomInfo {
      const std::ptrdiff_t block_index;
      const size_t block_size;

      explicit RomInfo(const std::ptrdiff_t index, const size_t size) : block_index(index), block_size(size) {};
    };

    struct RamInfo {
      const Argument::ptr &write_enable;
      const Argument::ptr &write_addr;
      const Argument::ptr &data;
      const std::ptrdiff_t block_index;
      const size_t block_size;

      explicit RamInfo(const Argument::ptr &write_enable,
                       const Argument::ptr &write_addr,
                       const Argument::ptr &data,
                       const std::ptrdiff_t index,
                       const size_t size)
          : write_enable(write_enable), write_addr(write_addr), data(data), block_index(index), block_size(size) {};
    };

    explicit MemoryMapper(const Program::ptr &p);

    void visit_reg_expr(const RegExpression &expr) override;
    void visit_rom_expr(const RomExpression &expr) override;
    void visit_ram_expr(const RamExpression &expr) override;

    Variable::ptr current_var;
    size_t current_index;

    // var = REG(o)
    // reg_info = { o -> (loc of o in memory) }
    std::unordered_map<Variable::ptr, size_t> reg_info;
    std::unordered_map<ident_t, RomInfo> rom_info;
    std::unordered_map<ident_t, RamInfo> ram_info;
  };

  class ExpressionEvaluator : public Visitor<MemoryMapper> {
  public:
    explicit ExpressionEvaluator(const var_env &env, const std::vector<value_t> &mem, const MemoryMapper &mem_map);

    [[nodiscard]] value_t eval(const Variable::ptr &var, const Expression::ptr &expr) noexcept;

    void visit_constant(const Constant::ptr &cst) override;
    void visit_variable(const Variable::ptr &var) override;

    void visit_arg_expr(const ArgExpression &expr) override;
    void visit_reg_expr(const RegExpression &expr) override;
    void visit_not_expr(const NotExpression &expr) override;
    void visit_binop_expr(const BinOpExpression &expr) override;
    void visit_mux_expr(const MuxExpression &expr) override;
    void visit_rom_expr(const RomExpression &expr) override;
    void visit_ram_expr(const RamExpression &expr) override;
    void visit_concat_expr(const ConcatExpression &expr) override;
    void visit_slice_expr(const SliceExpression &expr) override;
    void visit_select_expr(const SelectExpression &expr) override;
  private:
    const var_env &env;
    const std::vector<value_t> &mem;
    const MemoryMapper &mem_map;

    value_t current_value;
    Variable::ptr current_var;
  } expr_eval;

  [[nodiscard]] value_t eval_arg(const Argument::ptr &arg) const noexcept;

  const Program::ptr &prog;
  const ReportContext &ctx;
  const Scheduler::VariableList dep_list;

  const MemoryMapper mem_map;
  std::vector<value_t> memory;

  var_env env;
};

#endif //NETLIST_SRC_SIMULATOR_HPP
