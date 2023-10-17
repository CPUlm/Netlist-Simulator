#ifndef NETLIST_LEXER_HPP
#define NETLIST_LEXER_HPP

#include "token.hpp"

class DataBuffer {
public:
  explicit DataBuffer(const char *beginning) noexcept: line(0), col(0), current(beginning) {}

  void next_char();

  [[nodiscard]] char current_char() const { return *current; }

  [[nodiscard]] const char *current_pos() const { return current; }

  [[nodiscard]] uint32_t current_line() const { return line; }

  [[nodiscard]] uint32_t current_column() const { return col; }

  [[nodiscard]] bool is_eof() const { return *current == '\0'; }

private:
  uint32_t line;
  uint32_t col;
  const char *current;
};

/// The (super-simple) lexical analyser for the netlist language.
///
/// This class converts a sequence of bytes (representing the source code in
/// the ASCII or UTF-8 encoding) into a stream of Tokens.
class Lexer {
public:
  explicit Lexer(const char *input);

  /// Returns the next scanned token in the source code and advances the
  /// internal position of the lexer.
  /// When the end of input is reached then a EOI (End-Of-Input) token is
  /// generated, and all further calls will do the same.
  void tokenize(Token &token);

private:
  /// Just skip eagerly any whitespace found at the current position.
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
  DataBuffer buf;
};

#endif // NETLIST_LEXER_HPP
