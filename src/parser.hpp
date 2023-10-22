#ifndef NETLIST_PARSER_HPP
#define NETLIST_PARSER_HPP

#include <charconv>
#include <set>

#include "program.hpp"
#include "report.hpp"
#include "lexer.hpp"

class Parser {
public:
  explicit Parser(ReportContext &context, Lexer &lexer);

  /// Parse a program.
  [[nodiscard]] std::unique_ptr<Program> parse_program();

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
  typedef std::unordered_map<std::string_view, VariableReference> var_ref_map;

  /// Consumes the current token and gets the next one.
  void consume();

  /// Check that the current token is in token_set.
  /// If not, print an error.
  void assert(const std::set<TokenKind> &token_set) const noexcept;

  /// Check that the current token is token.
  /// If not, print an error.
  void assert(TokenKind token) const noexcept;

  /// Parse the current token as an integer in the given base. We use a class to process different integer types.
  template<class T>
  [[nodiscard]] T parse_int(int base) noexcept {
    assert({TokenKind::INTEGER,
            TokenKind::BINARY_CONSTANT,
            TokenKind::DECIMAL_CONSTANT,
            TokenKind::HEXADECIMAL_CONSTANT});
    T v;

    auto [ptr, ec] = std::from_chars(m_token.spelling.begin(), m_token.spelling.end(), v, base);
    if (ec != std::errc()) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message("The value '{}' (interpreted in base {}) is too big to fit in a {}."
                        " Max value authorised : '{}'",
                        m_token.spelling, base, typeid(T).name(), std::numeric_limits<T>::max())
          .build()
          .exit();
    } else if (*ptr != '\0') {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message("Error parsing value '{}' in base {}. Successfully parse the value '{}' but it remain '{}'",
                        m_token.spelling, base, v, ptr)
          .build()
          .exit();
    }

    consume(); // eat the integer
    return v;
  }

  /// Parse the INPUT statement
  [[nodiscard]] var_ref_map parse_inputs_references() noexcept;

  /// Parse the OUTPUT statement
  [[nodiscard]] var_ref_map parse_outputs_references() noexcept;

  /// Parse the VAR statement
  void parse_variable_declaration() noexcept;

  /// Build the 'input' and 'output' list in the program 'p'
  void build_intput_output_list(Program::ptr &p, const var_ref_map &in_refs, const var_ref_map &out_refs);

  /// Parse equations (everything after the 'IN' statement)
  void parse_equations(Program::ptr &p);

  /// Check that the variable 'var' has the size 'size'. Raise an error at the position 'pos'
  /// if 'var' has the wrong size.
  void assert_var_size(const Variable::ptr &var, bus_size_t size, const SourcePosition &pos) noexcept;

  /// Check that the variable 'var' has a size >= 'size'. If not, raise an error at the position 'pos'
  void assert_var_size_greater_than(const Variable::ptr &var, bus_size_t minimal_size,
                                    const SourcePosition &pos) noexcept;

  /// Parse a bus size.
  [[nodiscard]] bus_size_t parse_bus_size() noexcept;

  /// Parse a Binary Constant with size 'size'
  [[nodiscard]] Constant::ptr parse_bin_const(bus_size_t size) noexcept;

  /// Parse a Decimal Constant with size 'size'
  [[nodiscard]] Constant::ptr parse_dec_const(bus_size_t size) noexcept;

  /// Parse a Hexadecimal Constant with size 'size'
  [[nodiscard]] Constant::ptr parse_hex_const(bus_size_t size) noexcept;

  /// Parse a Variable
  [[nodiscard]] Variable::ptr parse_var() noexcept;

  /// Parse a Variable and check that it's size is 'size'
  [[nodiscard]] Variable::ptr parse_var(bus_size_t size) noexcept;

  /// Parse an Argument : a Variable or a Constant (Binary, Decimal or Hexadecimal)
  /// Check that the argument fit in a bus size of 'size'
  [[nodiscard]] Argument::ptr parse_argument(bus_size_t expected_size) noexcept;

  ReportContext &m_context;
  Lexer &m_lexer;
  Token m_token;

  /// mapping of var name to the actual variable class.
  std::unordered_map<std::string_view, Variable::ptr> vars;
};

#endif // NETLIST_PARSER_HPP
