#include "program.hpp"

std::vector<std::string_view> Program::get_input_names() const {
  std::vector<std::string_view> names;
  for (auto *input : m_inputs) {
    names.push_back(input->get_name());
  }

  return names;
}

std::vector<std::string_view> Program::get_output_names() const {
  std::vector<std::string_view> names;
  for (auto *equation : m_equations) {
    if (equation->is_output())
      names.push_back(equation->get_name());
  }

  return names;
}

Constant *Program::create_constant(size_t value) {
  auto *ptr = new Constant(value);
  m_values.emplace_back(ptr);
  return ptr;
}

Input *Program::create_input(std::string_view name) {
  auto *ptr = new Input(name);
  m_values.emplace_back(ptr);
  m_inputs.push_back(ptr);
  return ptr;
}

Equation *Program::create_output(std::string_view name) {
  auto *ptr = new Equation(name);
  ptr->mark_as_output();
  m_values.emplace_back(ptr);
  m_equations.push_back(ptr);
  return ptr;
}

Equation *Program::create_equation(std::string_view name) {
  auto *ptr = new Equation(name);
  m_values.emplace_back(ptr);
  m_equations.push_back(ptr);
  return ptr;
}

NotExpression *Program::create_not_expr(Value *value) {
  auto *ptr = new NotExpression(value);
  m_expressions.emplace_back(ptr);
  return ptr;
}

BinaryExpression *Program::create_binary_expr(BinaryOp op, Value *lhs,
                                              Value *rhs) {
  auto *ptr = new BinaryExpression(op, lhs, rhs);
  m_expressions.emplace_back(ptr);
  return ptr;
}
