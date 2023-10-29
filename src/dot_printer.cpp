#include "dot_printer.hpp"
#include "fmt/format.h"

#include <sstream>

class ExpressionIterator : public Visitor<ExpressionIterator> {
public:
  [[nodiscard]] std::string get_string() const noexcept {
    return m_sstr.str();
  }

  void begin_var(const Variable::ptr &beg) {
    m_sstr.str(std::string());
    m_links.str(std::string());
    m_beg_var = beg;
  }

  [[nodiscard]] std::string end_var() const noexcept {
    return m_links.str();
  }

  void visit_constant(const Constant &cst) override {
    m_sstr << fmt::format("{:#b}", cst.get_value());
  }

  void visit_variable(const Variable &var) override {
    m_sstr << var.get_name();
    m_links << "\t" << var.get_name() << " -> " << m_beg_var->get_name() << "\n";
  }

  void visit_arg_expr(const ArgExpression &expr) override {
    visit(expr.get_argument());
  }

  void visit_reg_expr(const RegExpression &expr) override {
    m_sstr << "REG(";
    visit(expr.get_variable());
    m_sstr << ")";
  }

  void visit_not_expr(const NotExpression &expr) override {
    m_sstr << "NOT ";
    visit(expr.get_argument());
  }

  void visit_binop_expr(const BinOpExpression &expr) override {
    switch (expr.get_binop_kind()) {
    case BinOpExpression::BinOpKind::OR:
      m_sstr << "OR (";
      break;
    case BinOpExpression::BinOpKind::XOR:
      m_sstr << "XOR (";
      break;
    case BinOpExpression::BinOpKind::AND:
      m_sstr << "AND (";
      break;
    case BinOpExpression::BinOpKind::NAND:
      m_sstr << "NAND (";
      break;
    }

    visit(expr.get_lhs_argument());
    m_sstr << ", ";
    visit(expr.get_rhs_argument());
    m_sstr << ")";
  }

  void visit_mux_expr(const MuxExpression &expr) override {
    m_sstr << "MUX (";
    visit(expr.get_choice_argument());
    m_sstr << ", ";
    visit(expr.get_true_argument());
    m_sstr << ", ";
    visit(expr.get_false_argument());
    m_sstr << ")";
  }

  void visit_rom_expr(const RomExpression &expr) override {
    m_sstr << "ROM (" << expr.get_address_size() << ", " << expr.get_bus_size() << ", ";
    visit(expr.get_read_address());
    m_sstr << ")";
  }

  void visit_ram_expr(const RamExpression &expr) override {
    m_sstr << "RAM (" << expr.get_address_size() << ", " << expr.get_bus_size() << ", ";
    visit(expr.get_read_address());
    m_sstr << ", ";
    visit(expr.get_write_enable());
    m_sstr << ", ";
    visit(expr.get_write_address());
    m_sstr << ", ";
    visit(expr.get_write_data());
    m_sstr << ")";
  }

  void visit_concat_expr(const ConcatExpression &expr) override {
    m_sstr << "CONCAT (";
    visit(expr.get_beginning_part());
    m_sstr << ", ";
    visit(expr.get_last_part());
    m_sstr << ")";
  }

  void visit_slice_expr(const SliceExpression &expr) override {
    m_sstr << "SLICE (" << expr.get_begin_index() << ", " << expr.get_end_index() << ", ";
    visit(expr.get_argument());
    m_sstr << ")";
  }

  void visit_select_expr(const SelectExpression &expr) override {
    m_sstr << "SELECT (" << expr.get_index() << ", ";
    visit(expr.get_argument());
    m_sstr << ")";

  }

private:
  std::stringstream m_sstr;
  std::stringstream m_links;
  Variable::ptr m_beg_var;
};

void DotPrinter::print(std::ostream &out) {
  ExpressionIterator expIt;

  out << "digraph {\n";

  for (const Variable::ptr &in_var : m_prog->get_inputs()) {
    out << "\t" << in_var->get_name() << " [label=<<b>" << in_var->get_name() << "</b><br/><i>size</i>: "
        << in_var->get_bus_size() << ">";
    if (m_prog->get_outputs().contains(in_var)) {
      out << ", shape=rect]\n";
    } else {
      out << "]\n";
    }
  }

  for (const auto &[var, eq] : m_prog->get_equations()) {
    expIt.begin_var(var);
    expIt.visit(eq);

    out << "\t" << var->get_name() << " [label=<" << var->get_name() << "<br/><i>size</i>: "
        << var->get_bus_size()
        << "<br/><i>eq</i>: " << expIt.get_string() << ">";

    if (m_prog->get_outputs().contains(var)) {
      out << ", shape=rect]\n";
    } else {
      out << "]\n";
    }

    out << expIt.end_var() << "\n";
  }

  out << "}\n";
}
