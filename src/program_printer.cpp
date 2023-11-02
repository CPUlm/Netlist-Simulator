#include "program_printer.hpp"
#include "fmt/format.h"

static void print_variable_reference(std::ostream &out, const std::unordered_set<Variable::ptr> &var_list) {
  bool is_first = true;

  for (const Variable::ptr &v : var_list) {
    if (is_first) {
      out << v->get_name();
      is_first = false;
    } else {
      out << ", " << v->get_name();
    }
  }
}

inline void print_var_decl(std::ostream &out, const Variable::ptr &var) {
  out << var->get_name();

  if (var->get_bus_size() > 1) {
    out << ":" << var->get_bus_size();
  }
}

static void print_variable_declaration(std::ostream &out, const Program::ptr &p) {
  bool is_first = true;

  for (const Variable::ptr &v : p->get_inputs()) { // Print INPUT variables
    if (is_first) {
      is_first = false;
    } else {
      out << ", ";
    }

    print_var_decl(out, v);
  }

  for (const auto &eq_s : p->get_equations()) {
    const Variable::ptr &v = eq_s.first;
    if (is_first) {
      is_first = false;
    } else {
      out << ", ";
    }

    print_var_decl(out, v);
  }
}

ProgramPrinter::ProgramPrinter(const Program::ptr &program, std::ostream &out) : p(program), out(out) {}

void ProgramPrinter::print() const {
  out << "INPUT ";
  print_variable_reference(out, p->get_inputs());
  out << "\n";

  out << "OUTPUT ";
  print_variable_reference(out, p->get_outputs());
  out << "\n";

  out << "VAR ";
  print_variable_declaration(out, p);
  out << "\n";

  out << "IN\n";
  for (const auto &pair : p->get_equations()) {
    const Expression::ptr &expr = pair.second;
    out << pair.first->get_name() << " = ";
    visit(expr);
    out << "\n";
  }
}

void ProgramPrinter::visit_constant(const Constant::ptr &cst) const {
  out << fmt::format("{:b}", cst->get_value());
}

void ProgramPrinter::visit_variable(const Variable::ptr &var) const {
  out << var->get_name();
}

void ProgramPrinter::visit_arg_expr(const ArgExpression &expr) const {
  visit(expr.get_argument());
}

void ProgramPrinter::visit_reg_expr(const RegExpression &expr) const {
  out << "REG ";
  visit(expr.get_variable());
}

void ProgramPrinter::visit_not_expr(const NotExpression &expr) const {
  out << "NOT ";
  visit(expr.get_argument());
}

void ProgramPrinter::visit_binop_expr(const BinOpExpression &expr) const {
  switch (expr.get_binop_kind()) {

  case BinOpExpression::BinOpKind::OR:
    out << "OR ";
    break;
  case BinOpExpression::BinOpKind::XOR:
    out << "XOR ";
    break;
  case BinOpExpression::BinOpKind::AND:
    out << "AND ";
    break;
  case BinOpExpression::BinOpKind::NAND:
    out << "NAND ";
    break;
  }

  visit(expr.get_lhs_argument());
  out << " ";
  visit(expr.get_rhs_argument());
}

void ProgramPrinter::visit_mux_expr(const MuxExpression &expr) const {
  out << "MUX ";
  visit(expr.get_choice_argument());
  out << " ";
  visit(expr.get_true_argument());
  out << " ";
  visit(expr.get_false_argument());
}

void ProgramPrinter::visit_rom_expr(const RomExpression &expr) const {
  out << "ROM " << expr.get_address_size() << " " << expr.get_bus_size() << " ";
  visit(expr.get_read_address());
}

void ProgramPrinter::visit_ram_expr(const RamExpression &expr) const {
  out << "RAM " << expr.get_address_size() << " " << expr.get_bus_size() << " ";
  visit(expr.get_read_address());
  out << " ";
  visit(expr.get_write_enable());
  out << " ";
  visit(expr.get_write_address());
  out << " ";
  visit(expr.get_write_data());
}

void ProgramPrinter::visit_concat_expr(const ConcatExpression &expr) const {
  out << "CONCAT ";
  visit(expr.get_beginning_part());
  out << " ";
  visit(expr.get_last_part());
}

void ProgramPrinter::visit_slice_expr(const SliceExpression &expr) const {
  out << "SLICE " << expr.get_begin_index() << " " << expr.get_end_index() << " ";
  visit(expr.get_argument());
}

void ProgramPrinter::visit_select_expr(const SelectExpression &expr) const {
  out << "SELECT " << expr.get_index() << " ";
  visit(expr.get_argument());
}
