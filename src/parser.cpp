#include "parser.hpp"

#include <cassert>
#include <format>
#include <vector>

[[nodiscard]] static size_t parse_integer_literal(std::string_view literal) {
  size_t value = 0;
  for (char ch : literal) {
    value *= 10;
    value += ch - '0';
  }

  return value;
}

Parser::Parser(ReportManager &report_manager, Lexer &lexer)
    : m_report_manager(report_manager), m_lexer(lexer) {
  // Gets the first token
  m_lexer.tokenize(m_token);
}

Program Parser::parse_program() {
  auto inputs = parse_inputs();
  auto outputs = parse_outputs();
  auto variables = parse_variables();

  create_named_values(inputs, outputs, variables);

  parse_equations();

  return std::move(m_program);
}

std::vector<std::string_view> Parser::parse_inputs() {
  if (!expect(TokenKind::KEY_INPUT))
    return {};

  consume(); // eat `INPUT`

  return parse_variable_list();
}

std::vector<std::string_view> Parser::parse_outputs() {
  if (!expect(TokenKind::KEY_OUTPUT))
    return {};

  consume(); // eat `OUTPUT`

  return parse_variable_list();
}

std::vector<std::string_view> Parser::parse_variables() {
  if (!expect(TokenKind::KEY_VAR))
    return {};

  consume(); // eat `VAR`

  return parse_variable_list(/* accept_size_specifiers= */ true);
}

std::vector<std::string_view>
Parser::parse_variable_list(bool accept_size_specifiers) {
  if (m_token.kind != TokenKind::IDENTIFIER)
    return {};

  std::vector<std::string_view> variables;

  do {
    if (!expect(TokenKind::IDENTIFIER)) {
      return variables;
    }

    VariableInfo variable_info = {};
    variable_info.spelling = m_token.spelling;
    variable_info.source_location = m_token.position;
    variables.push_back(variable_info.spelling);
    consume(); // consume the identifier

    // Parse the optional size specifier for variables.
    if (accept_size_specifiers && m_token.kind == TokenKind::COLON) {
      consume(); // eat `:`

      if (m_token.kind == TokenKind::INTEGER) {
        variable_info.size_in_bits = parse_integer_literal(m_token.spelling);
        consume(); // eat the integer literal
      } else {
        m_report_manager.report(ReportSeverity::ERROR)
            .with_location(m_token.position)
            .with_message("missing size in bits of the variable after the `:'")
            .finish()
            .print();
      }
    }

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat `,`
      continue;
    } else {
      return variables;
    }
  } while (true);
}

void Parser::create_named_values(
    const std::vector<std::string_view> &inputs,
    const std::vector<std::string_view> &outputs,
    const std::vector<std::string_view> &variables) {
  // Algorithm overview:
  // For each variable in variables:
  //   If variable is in inputs:
  //     create an input
  //   Otherwise:
  //     create an equation
  //     mark the created equation as an output if variable is in outputs

  for (const auto variable : variables) {
    const bool is_input =
        std::find(inputs.begin(), inputs.end(), variable) != inputs.end();
    const bool is_output =
        std::find(outputs.begin(), outputs.end(), variable) != outputs.end();

    Value *named_value = nullptr;
    if (is_input) {
      named_value = m_program.create_input(variable);
    } else if (is_output) {
      named_value = m_program.create_output(variable);
    } else {
      named_value = m_program.create_equation(variable);
    }

    // TODO: is really an error to be both input and output?
    if (is_input && is_output) {
      // TODO: give source location to the error message
      m_report_manager.report(ReportSeverity::ERROR)
          .with_message("the variable `{}' is declared as "
                        "input and output at the same time",
                        variable)
          .finish()
          .print();
    }

    const auto it = m_named_values.find(variable);
    if (it == m_named_values.end()) {
      m_named_values.insert({variable, named_value});
      continue;
    }

    // TODO: give source location to the error message
    m_report_manager.report(ReportSeverity::ERROR)
        .with_message("the variable `{}' is declared more "
                      "than once in the `VAR' statement",
                      variable)
        .finish()
        .print();
  }

  // Check if all inputs were declared in the `VAR` statement
  for (const auto input : inputs) {
    const auto it = m_named_values.find(input);
    if (it == m_named_values.end()) {
      // TODO: give source location to the error message
      emit_unknown_variable_error(INVALID_LOCATION, input);
    }
  }

  // Check if all outputs were declared in the `VAR` statement
  for (const auto output : outputs) {
    const auto it = m_named_values.find(output);
    if (it == m_named_values.end()) {
      // TODO: give source location to the error message
      emit_unknown_variable_error(INVALID_LOCATION, output);
    }
  }
}

void Parser::parse_equations() {
  // We expect the `IN` keyword, but if it's absent we just pretend it's there.
  if (expect(TokenKind::KEY_IN))
    consume(); // eat `IN`

  while (m_token.kind != TokenKind::EOI) {
    Equation *equation = parse_equation();
    if (equation == nullptr)
      return;
  }
}

Equation *Parser::parse_equation() {
  if (!expect(TokenKind::IDENTIFIER)) {
    return nullptr;
  }

  const auto name_location = m_token.position;
  const auto name = m_token.spelling;
  consume(); // eat the identifier

  Equation *equation = nullptr;
  const auto it = m_named_values.find(name);
  if (it == m_named_values.end()) {
    emit_unknown_variable_error(name_location, name);
  } else {
    Value *value = it->second;
    if (!value->is_input()) {
      equation = static_cast<Equation *>(value);
    } else {

      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(name_location)
          .with_message(
              "cannot assign an equation to an input variable", name)
          .finish()
          .print();
    }
  }

  if (!expect(TokenKind::EQUAL)) {
    return nullptr;
  }

  consume(); // eat `=`

  Expression *expression = parse_expression();
  if (expression == nullptr)
    return nullptr;

  if (equation != nullptr) {
    equation->set_expression(expression);
  }

  return equation;
}

Value *Parser::parse_argument() {
  switch (m_token.kind) {
  case TokenKind::IDENTIFIER:
    return parse_variable();
  case TokenKind::INTEGER:
    return parse_constant();
  default:
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message(
            "unexpected token, expected either an identifier or a constant")
        .finish()
        .print();
    return nullptr;
  }
}

Value *Parser::parse_variable() {
  assert(m_token.kind == TokenKind::IDENTIFIER);

  const auto name_location = m_token.position;
  const auto name = m_token.spelling;
  consume();

  const auto it = m_named_values.find(name);
  if (it != m_named_values.end()) {
    return it->second;
  }

  emit_unknown_variable_error(name_location, name);
  return nullptr;
}

Constant *Parser::parse_constant() {
  assert(m_token.kind == TokenKind::INTEGER);

  const size_t value = parse_integer_literal(m_token.spelling);
  consume();

  return m_program.create_constant(value);
}

Expression *Parser::parse_expression() {
  switch (m_token.kind) {
  case TokenKind::KEY_NOT:
    return parse_not_expression();
  case TokenKind::KEY_AND:
  case TokenKind::KEY_OR:
  case TokenKind::KEY_NAND:
  case TokenKind::KEY_XOR:
    return parse_binary_expression();
  default:
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("invalid expression, expected an operator or a constant")
        .finish()
        .print();
    return nullptr;
  }
}

NotExpression *Parser::parse_not_expression() {
  assert(m_token.kind == TokenKind::KEY_NOT);

  consume(); // eat `NOT`

  Value *value = parse_argument();
  return m_program.create_not_expr(value);
}

BinaryExpression *Parser::parse_binary_expression() {
  BinaryOp binop;
  switch (m_token.kind) {
  case TokenKind::KEY_AND:
    binop = BinaryOp::AND;
    break;
  case TokenKind::KEY_OR:
    binop = BinaryOp::OR;
    break;
  case TokenKind::KEY_NAND:
    binop = BinaryOp::NAND;
    break;
  case TokenKind::KEY_XOR:
    binop = BinaryOp::XOR;
    break;
  default:
    assert(false && "expected a binary operator");
    return nullptr;
  }

  consume(); // eat the binary operator

  Value *lhs = parse_argument();
  Value *rhs = parse_argument();
  return m_program.create_binary_expr(binop, lhs, rhs);
}

void Parser::consume() { m_lexer.tokenize(m_token); }

bool Parser::expect(TokenKind token_kind) const {
  if (m_token.kind == token_kind)
    return true;

  // FIXME(hgruniaux): emit an error
  return false;
}

void Parser::emit_unknown_variable_error(SourceLocation location,
                                         std::string_view variable_name) {

  m_report_manager.report(ReportSeverity::ERROR)
      .with_location(location)
      .with_message(
          "the variable `{}' is used but not declared in the `VAR' statement",
          variable_name)
      .finish()
      .print();
}
