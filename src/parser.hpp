#ifndef NETLIST_PARSER_HPP
#define NETLIST_PARSER_HPP

#include <memory>
#include <set>
#include <charconv>

#include "program.hpp"
#include "lexer.hpp"
#include "report.hpp"

class Parser {
public:
  explicit Parser(ReportContext &context, Lexer &lexer);

  [[nodiscard]] std::unique_ptr<Program> parse_program();

private:
  /// Consumes the current token and gets the next one.
  void consume();

  /// Check that the current token is in token_set.
  /// If not, print an error.
  void assert(const std::set<TokenKind> &token_set) const noexcept;

  /// Check that the current token is token.
  /// If not, print an error.
  void assert(TokenKind token) const noexcept;

private:
  ReportContext &m_context;
  Lexer &m_lexer;
  Token m_token;

  struct VariableDeclaration {
    std::string_view spelling;
    SourcePosition position;
    bus_size_t size;
  };

  struct VariableReference {
    std::string_view spelling;
    SourcePosition position;
  };

  template<class T>
  [[nodiscard]] T parse_integer() noexcept {
    assert(TokenKind::INTEGER);
    T v;

    auto res = std::from_chars(m_token.spelling.begin(), m_token.spelling.end(), v);
    if (res.ec != std::errc()) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message("Integer '{}' is too big to fit in a {}. Max value authorised : '{}'",
                        m_token.spelling, typeid(T).name(), std::numeric_limits<T>::max())
          .build()
          .exit();
    }

    consume(); // eat the integer
    return v;
  }

  std::vector<VariableReference> parse_inputs() noexcept;
  std::vector<VariableReference> parse_outputs() noexcept;
  std::vector<Parser::VariableDeclaration> parse_var_decl() noexcept;
};

#endif // NETLIST_PARSER_HPP
