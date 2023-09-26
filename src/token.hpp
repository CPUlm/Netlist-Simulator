#ifndef NETLIST_TOKEN_HPP
#define NETLIST_TOKEN_HPP

enum class TokenKind {
  // Main tokens
  /// End-Of-Input, the last token returned by the lexer.
  EOI,
  /// An identifier such as `x` or `t_1`.
  IDENTIFIER,
  /// An equal sign `=`.
  EQUAL,
  /// A comma `,`.
  COMMA,

// Keywords
#define KEYWORD(spelling) KEY_##spelling,
#include "keywords.def"
};

struct Token {
  TokenKind kind = TokenKind::EOI;
};

#endif // NETLIST_TOKEN_HPP
