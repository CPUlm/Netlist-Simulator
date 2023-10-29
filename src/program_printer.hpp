#ifndef NETLIST_PROGRAM_PRINTER_HPP
#define NETLIST_PROGRAM_PRINTER_HPP

#include <iostream>

#include "program.hpp"

/// This class prints the given netlist program to the standard output
/// using the netlist syntax. Therefore, the parser should be able to
/// parse back the output of this class into the same program.
class ProgramPrinter : public Visitor<ProgramPrinter> {
public:
  explicit ProgramPrinter(const Program::ptr &program, std::ostream &out = std::cout);

  /// Prints the given program to the given output. THIS IS THE ONLY FUNCTION TO USE.
  void print();

  void visit_constant(const Constant &cst) override;
  void visit_variable(const Variable &var) override;
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
  const Program::ptr &p;
  std::ostream &out;
};

#endif // NETLIST_PROGRAM_PRINTER_HPP
