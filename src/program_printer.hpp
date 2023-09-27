#ifndef NETLIST_PROGRAM_PRINTER_HPP
#define NETLIST_PROGRAM_PRINTER_HPP

#include "program.hpp"

/// This class prints the given netlist program to the standard output
/// using the netlist syntax. Therefore, the parser should be able to
/// parse back the output of this class into the same program.
///
/// The real public method of this class is print_program(). You should not
/// use the other ones.
class ProgramPrinter : public Visitor<ProgramPrinter> {
public:
  /// Prints the given program to the standard output.
  void print_program(const Program &program);

  void print_equations(const std::vector<Equation *> &equations);
  void print_value(Value *value);

  void visit_value(Value *value);
  void visit_constant(Constant *value);

  void visit_not_expr(NotExpression *expr);
  void visit_binary_expr(BinaryExpression *expr);
};

#endif // NETLIST_PROGRAM_PRINTER_HPP
