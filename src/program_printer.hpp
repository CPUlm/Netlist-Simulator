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
  void print() const;

  void visit_constant(const Constant &cst) const override;
  void visit_variable(const Variable &var) const override;
  void visit_arg_expr(const ArgExpression &expr) const override;
  void visit_reg_expr(const RegExpression &expr) const override;
  void visit_not_expr(const NotExpression &expr) const override;
  void visit_binop_expr(const BinOpExpression &expr) const override;
  void visit_mux_expr(const MuxExpression &expr) const override;
  void visit_rom_expr(const RomExpression &expr) const override;
  void visit_ram_expr(const RamExpression &expr) const override;
  void visit_concat_expr(const ConcatExpression &expr) const override;
  void visit_slice_expr(const SliceExpression &expr) const override;
  void visit_select_expr(const SelectExpression &expr) const override;

private:
  const Program::ptr &p;
  std::ostream &out;
};

#endif // NETLIST_PROGRAM_PRINTER_HPP
