#ifndef NETLIST_PARSER_HPP
#define NETLIST_PARSER_HPP

#include "lexer.hpp"
#include "program.hpp"

#include <unordered_map>
#include <unordered_set>

class Parser {
public:
  explicit Parser(Lexer &lexer);

  [[nodiscard]] Program parse_program();

private:
  /// Consumes the current token and gets the next one.
  void consume();
  bool expect(TokenKind token_kind) const;

  [[nodiscard]] std::vector<std::string_view> parse_inputs();
  [[nodiscard]] std::vector<std::string_view> parse_outputs();
  [[nodiscard]] std::vector<std::string_view> parse_variables();
  [[nodiscard]] std::vector<std::string_view> parse_variable_list();

  void create_named_values(const std::vector<std::string_view> &inputs,
                           const std::vector<std::string_view> &outputs,
                           const std::vector<std::string_view> &variables);

  void parse_equations();
  Equation *parse_equation();
  [[nodiscard]] Value *parse_argument();
  [[nodiscard]] Value *parse_variable();
  [[nodiscard]] Constant *parse_constant();
  [[nodiscard]] Expression *parse_expression();
  [[nodiscard]] NotExpression *parse_not_expression();
  [[nodiscard]] BinaryExpression *parse_binary_expression();

private:
  Lexer &m_lexer;
  Token m_token;
  // Mapping between the name and the values. Named values include
  // inputs and equations (which may also be outputs).
  std::unordered_map<std::string_view, Value *> m_named_values;
  Program m_program;
};

#endif // NETLIST_PARSER_HPP
