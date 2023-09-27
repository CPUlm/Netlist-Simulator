#include "program_printer.hpp"

#include <iostream>

static void
print_comma_separated_list(const std::vector<std::string_view> &list) {
  bool is_first = true;
  for (const auto &item : list) {
    if (!is_first) {
      std::cout << ", ";
    } else {
      is_first = false;
    }

    std::cout << item;
  }
}

void ProgramPrinter::print_program(const Program &program) {
  std::cout << "INPUT ";
  print_comma_separated_list(program.get_input_names());
  std::cout << "\n";

  std::cout << "OUTPUT ";
  print_comma_separated_list(program.get_output_names());
  std::cout << "\n";

  std::cout << "VAR ";
  bool is_first_var = true;
  for (const auto &value : program.get_values()) {
    if (!value->is_named())
      continue;

    if (!is_first_var) {
      std::cout << ", ";
    } else {
      is_first_var = false;
    }

    std::cout << value->get_name() << ":" << value->get_size_in_bits();
  }
  std::cout << "\n";

  std::cout << "IN\n";
  print_equations(program.get_equations());
}

void ProgramPrinter::visit_value(Value *value) {
  if (value->get_name().empty())
    std::cout << "@UNNAMED";
  else
    std::cout << value->get_name();
}

void ProgramPrinter::visit_constant(Constant *value) {
  std::cout << value->get_value();
}

void ProgramPrinter::visit_not_expr(NotExpression *expr) {
  std::cout << "NOT ";
  print_value(expr->get_value());
}

void ProgramPrinter::visit_binary_expr(BinaryExpression *expr) {
  switch (expr->get_operator()) {
  case BinaryOp::AND:
    std::cout << "AND ";
    break;
  case BinaryOp::OR:
    std::cout << "OR ";
    break;
  case BinaryOp::NAND:
    std::cout << "NAND ";
    break;
  case BinaryOp::XOR:
    std::cout << "XOR ";
    break;
  }

  print_value(expr->get_lhs());
  std::cout << " ";
  print_value(expr->get_rhs());
}

void ProgramPrinter::print_value(Value *value) {
  if (value == nullptr)
    std::cout << "@ERROR";
  else
    visit(value);
}

void ProgramPrinter::print_equations(const std::vector<Equation *> &equations) {
  for (auto *equation : equations) {
    std::cout << equation->get_name() << " = ";

    if (equation->get_expression() != nullptr) {
      visit(equation->get_expression());
    } else {
      std::cout << "@ERROR";
    }

    std::cout << "\n";
  }
}
