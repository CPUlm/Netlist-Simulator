#ifndef NETLIST_LEXER_HPP
#define NETLIST_LEXER_HPP

#include "token.hpp"
#include "report.hpp"

class DataBuffer {
public:
  explicit DataBuffer(const char *beginning) noexcept: m_line(0), m_col(0), m_cur(beginning) {}

  void next_char();

  [[nodiscard]] char current_char() const { return *m_cur; }

  [[nodiscard]] const char *current_pos() const { return m_cur; }

  [[nodiscard]] uint32_t current_line() const { return m_line; }

  [[nodiscard]] uint32_t current_column() const { return m_col; }

  [[nodiscard]] bool is_eof() const { return *m_cur == '\0'; }

private:
  uint32_t m_line;
  uint32_t m_col;
  const char *m_cur;
};

/// The (super-simple) lexical analyser for the netlist language.
///
/// This class converts a sequence of bytes (representing the source code in
/// the ASCII or UTF-8 encoding) into a stream of Tokens.
class Lexer {
public:
  explicit Lexer(ReportContext &context, const char *input);

  /// Returns the next scanned token in the source code and advances the
  /// internal position of the lexer.
  /// When the end of input is reached then a EOI (End-Of-Input) token is
  /// generated, and all further calls will do the same.
  void tokenize(Token &token);

private:
  /// Just skip eagerly any whitespace found at the m_cur position.
  void skip_whitespace();
  /// Just skip until the end of the line. This function should only be
  /// called when the lexer is located at the start of a comment.
  void skip_comment();
  /// Tokenizes an IDENTIFIER. This function should only be called when the
  /// lexer is located at the first valid character of an identifier.
  void tokenize_identifier(Token &token);
  /// Tokenizes an INTEGER. This function should only be called when the
  /// lexer is located at the first valid character of an integer.
  void tokenize_integer(Token &token);

private:
  DataBuffer m_buf;
  ReportContext &m_context;
};

#endif // NETLIST_LEXER_HPP
