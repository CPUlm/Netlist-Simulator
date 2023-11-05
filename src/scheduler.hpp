#ifndef NETLIST_SRC_SCHEDULER_HPP
#define NETLIST_SRC_SCHEDULER_HPP

#include "program.hpp"
#include "report.hpp"
#include "graph.hpp"

class Scheduler {
public:
  using VariableList = std::vector<Variable::ptr>;

  [[nodiscard]] static VariableList schedule(const ReportContext &ctx, const Program::ptr &p) noexcept {
    Scheduler s(ctx, p);
    return s.schedule_program();
  }

private:
  explicit Scheduler(const ReportContext &ctx, const Program::ptr &p) : m_ctx(ctx), m_prog(p) {};

  [[nodiscard]] VariableList schedule_program() noexcept {
    Graph<Variable::ptr> g;
    ExpressionIterator expIt(g, m_prog->get_inputs());

    for (const auto &[var, eq] : m_prog->get_equations()) {
      g.add_node(var);
      expIt.set_current_variable(var);
      expIt.visit(eq);
    }

    try {
      return g.topological();
    } catch (HasCycle &e) {
      m_ctx.report(ReportSeverity::ERROR)
          .with_code(40)
          .with_message("Cycle detected in NetList. Impossible to Schedule it.")
          .build()
          .exit();
    }
  }

  class ExpressionIterator : public Visitor<ExpressionIterator> {
  public:
    explicit ExpressionIterator(Graph<Variable::ptr> &g, const std::unordered_set<Variable::ptr> &inputs)
        : m_g(g), inputs(inputs) {};

    void set_current_variable(const Variable::ptr &var) {
      m_curr_var = var;
    }

    void visit_arg_expr(const ArgExpression &expr) override {
      add_dependency(expr.get_argument());
    }

    void visit_reg_expr(const RegExpression &expr) override {
      // REG break dependency, so we do nothing here.
    }

    void visit_not_expr(const NotExpression &expr) override {
      add_dependency(expr.get_argument());
    }

    void visit_binop_expr(const BinOpExpression &expr) override {
      add_dependency(expr.get_lhs_argument());
      add_dependency(expr.get_rhs_argument());
    }

    void visit_mux_expr(const MuxExpression &expr) override {
      add_dependency(expr.get_choice_argument());
      add_dependency(expr.get_true_argument());
      add_dependency(expr.get_false_argument());
    }

    void visit_rom_expr(const RomExpression &expr) override {
      add_dependency(expr.get_read_address());
    }

    void visit_ram_expr(const RamExpression &expr) override {
      add_dependency(expr.get_read_address()); // Writing occur at the end of the cycle, so no dependency introduced
    }

    void visit_concat_expr(const ConcatExpression &expr) override {
      add_dependency(expr.get_beginning_part());
      add_dependency(expr.get_last_part());
    }

    void visit_slice_expr(const SliceExpression &expr) override {
      add_dependency(expr.get_argument());
    }

    void visit_select_expr(const SelectExpression &expr) override {
      add_dependency(expr.get_argument());
    }

  private:
    void add_dependency(const Argument::ptr &arg) {
      if (arg->get_kind() == Argument::Kind::VARIABLE) {
        const Variable::ptr &var = std::static_pointer_cast<const Variable>(arg);
        if (!inputs.contains(var)) {
          m_g.add_edge(var, m_curr_var);
        }
      }
    }

    Graph<Variable::ptr> &m_g;
    const std::unordered_set<Variable::ptr> &inputs;
    Variable::ptr m_curr_var;
  };

  const ReportContext &m_ctx;
  const Program::ptr &m_prog;
};

#endif //NETLIST_SRC_SCHEDULER_HPP
