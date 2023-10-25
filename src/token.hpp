#ifndef NETLIST_TOKEN_HPP
#define NETLIST_TOKEN_HPP

#include <string_view>
#include <unordered_map>

enum class TokenKind {
  /// End-Of-Input, the last token returned by the lexer.
  EOI,
  /// An identifier such as 'x' or 't_1'.
  IDENTIFIER,
  /// An integer such as '42' or '01101110011101'.
  INTEGER,
  /// A constant such as '0b10111101'.
  BINARY_CONSTANT,
  /// A decimal constant '0d01245'.
  DECIMAL_CONSTANT,
  /// A hexadecimal constant '0x5fe89'.
  HEXADECIMAL_CONSTANT,
  /// An equal sign '='.
  EQUAL,
  /// A comma ','.
  COMMA,
  /// A colon ':'.
  COLON,
  /// The keyword 'OUTPUT'.
  KEY_OUTPUT,
  /// The keyword 'INPUT'.
  KEY_INPUT,
  /// The keyword 'VAR'.
  KEY_VAR,
  /// The keyword 'IN'.
  KEY_IN,
  /// The keyword 'NOT'.
  KEY_NOT,
  /// The keyword 'AND'.
  KEY_AND,
  /// The keyword 'NAND'.
  KEY_NAND,
  /// The keyword 'OR'.
  KEY_OR,
  /// The keyword 'XOR'.
  KEY_XOR,
  /// The keyword 'MUX'.
  KEY_MUX,
  /// The keyword 'REG'.
  KEY_REG,
  /// The keyword 'CONCAT'.
  KEY_CONCAT,
  /// The keyword 'SELECT'.
  KEY_SELECT,
  /// The keyword 'SLICE'.
  KEY_SLICE,
  /// The keyword 'ROM'.
  KEY_ROM,
  /// The keyword 'RAM'.
  KEY_RAM,
};

struct SourcePosition {
  uint32_t line;
  uint32_t begin;
};

/// A lexical unit of the source code such as an IDENTIFIER, a CONSTANT, a
/// COMMA, etc.
struct Token {
  /// The token's kind as classified by the lexer.
  TokenKind kind = TokenKind::EOI;
  /// The spelling of the token as found in the source code. The string view
  /// remains valid until the end of life of the input buffer given to the
  /// lexer.
  std::string_view spelling;
  /// The position of the token in the buffer.
  SourcePosition position;
};

#endif // NETLIST_TOKEN_HPP
