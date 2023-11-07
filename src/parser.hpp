#ifndef NETLIST_PARSER_HPP
#define NETLIST_PARSER_HPP

#include <set>
#include <optional>

#include "program.hpp"
#include "report.hpp"
#include "input_manager.hpp"
#include "lexer.hpp"

class Parser {
public:
  explicit Parser(const ReportContext &context, Lexer &lexer);

  /// Parse a program.
  [[nodiscard]] std::unique_ptr<Program> parse_program();

  /// Parse an Input File
  [[nodiscard]] InputManager::MemoryBlocks parse_input();

  /// Parse a value given by the user (stdin)
  [[nodiscard]] static value_t get_input_value(const Variable::ptr &var);
private:
  /// Variable declaration (used in the VAR statement)
  struct VariableDeclaration {
    std::string_view spelling;
    SourcePosition position;
    bus_size_t size;
  };

  /// Variable reference (only used to make a reference of a variable in the OUTPUT and INPUT statement)
  struct VariableReference {
    std::string_view spelling;
    SourcePosition position;
  };

  /// Type alias (because it's a very long type) for a dictionary that maps a variable name to it's reference.
  using var_ref_map = std::unordered_map<std::string_view, VariableReference>;

  /// Consumes the current token and gets the next one.
  void consume() { m_lexer.tokenize(m_token); }

  /// Check that the current token is in token_set.
  /// If not, print an error.
  void token_assert(const std::set<TokenKind> &token_set) const;

  /// Check that the current token is token.
  /// If not, print an error.
  void token_assert(TokenKind token) const;

  /// Parse the INPUT statement
  [[nodiscard]] var_ref_map parse_inputs_references();

  /// Parse the OUTPUT statement
  [[nodiscard]] var_ref_map parse_outputs_references();

  /// Parse the VAR statement
  void parse_variable_declaration();

  /// Build the 'input' and 'output' list in the program 'p'
  void build_intput_output_list(Program::ptr &p, const var_ref_map &in_refs, const var_ref_map &out_refs);

  /// Parse equations (everything after the 'IN' statement)
  void parse_equations(Program::ptr &p);

  /// Parse an expression
  [[nodiscard]] Expression::ptr parse_expression();

  /// Parse a bus size.
  [[nodiscard]] bus_size_t parse_bus_size();

  /// Parse a size specifier.
  [[nodiscard]] std::optional<bus_size_t> parse_size_spec();

  /// Parse Binary Digits
  [[nodiscard]] Constant::ptr parse_binary_digits();

  /// Parse a Binary Constant
  [[nodiscard]] Constant::ptr parse_binary_constant();

  /// Parse a Decimal Constant
  [[nodiscard]] Constant::ptr parse_decimal_constant();

  /// Parse a Hexadecimal Constant
  [[nodiscard]] Constant::ptr parse_hexadecimal_constant();

  /// Parse a Constant
  [[nodiscard]] Constant::ptr parse_constant();

  /// Parse a Variable
  [[nodiscard]] Variable::ptr parse_variable();

  /// Parse an Argument : a Variable or a Constant (Binary, Decimal or Hexadecimal)
  /// Check that the argument fit in a bus size of 'size'
  [[nodiscard]] Argument::ptr parse_argument();

  const ReportContext &m_context;
  Lexer &m_lexer;
  Token m_token;

  /// mapping of var name to the actual variable class.
  std::unordered_map<std::string_view, Variable::ptr> vars;

  /// mapping of var name to its declaration.
  std::unordered_map<std::string_view, Parser::VariableDeclaration> var_decl;

  void assert_same_bus_size(const Argument::ptr &arg1, const Argument::ptr &arg2, const SourcePosition &pos);

  void assert_bus_size_eq(const Argument::ptr &arg, bus_size_t size, const SourcePosition &pos);

  void assert_constant_size_eq(const Constant::ptr &arg, bus_size_t size, const SourcePosition &pos);

  void assert_bus_size_gt(const Argument::ptr &arg, bus_size_t size, const SourcePosition &pos);

  [[noreturn]] void unexpected_token(const std::set<TokenKind> &expected) const;
};

#endif // NETLIST_PARSER_HPP
