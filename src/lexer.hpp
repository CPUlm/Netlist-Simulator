#ifndef NETLIST_LEXER_HPP
#define NETLIST_LEXER_HPP

#include "token.hpp"

/// \ingroup parser
/// \brief The (super-simple) lexical analyser for the Netlist language.
///
/// This class converts a sequence of bytes (representing the source code in
/// the ASCII or UTF-8 encoding) into a stream of Token.
///
/// The Lexer is lazy, it only generates tokens as the user/parser request.
class Lexer {
public:
  explicit Lexer(const char *input);

  /// Returns the next scanned token in the source code and advances the
  /// internal position of the lexer.
  ///
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
  const char *m_input = nullptr;
  const char *m_cursor = nullptr;
};

#endif // NETLIST_LEXER_HPP
