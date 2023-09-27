#ifndef NETLIST_PROGRAM_PRINTER_HPP
#define NETLIST_PROGRAM_PRINTER_HPP

#include "program.hpp"

class ProgramPrinter : public Visitor<ProgramPrinter> {
public:
  void print_program(const Program& program);

  void visit_value(Value *value);
  void visit_constant(Constant *value);

  void visit_not_expr(NotExpression *expr);
  void visit_binary_expr(BinaryExpression *expr);

  void print_value(Value *value);
  void print_equations(const std::vector<Equation *> &equations);
};

#endif // NETLIST_PROGRAM_PRINTER_HPP
