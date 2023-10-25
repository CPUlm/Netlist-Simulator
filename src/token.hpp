#ifndef NETLIST_TOKEN_HPP
#define NETLIST_TOKEN_HPP

#include <string_view>

struct SourceLocation {
  uint32_t offset;

  [[nodiscard]] bool is_invalid() const {
    return offset == UINT32_MAX;
  }

  [[nodiscard]] static SourceLocation from_offset(uint32_t offset) {
    return { offset };
  }
};

struct SourceRange {
  SourceLocation location;
  uint32_t length;
};

static constexpr SourceLocation INVALID_LOCATION = { UINT32_MAX };

enum class TokenKind {
  /// End-Of-Input, the last token returned by the lexer.
  EOI,
  /// An identifier such as `x` or `t_1`.
  IDENTIFIER,
  /// An integer such as `42`.
  INTEGER,
  /// An equal sign `=`.
  EQUAL,
  /// A comma `,`.
  COMMA,
  /// A colon `:`.
  COLON,
  /// The keyword `OUTPUT`.
  KEY_OUTPUT,
  /// The keyword `INPUT`.
  KEY_INPUT,
  /// The keyword `VAR`.
  KEY_VAR,
  /// The keyword `IN`.
  KEY_IN,
  /// The keyword `NOT`.
  KEY_NOT,
  /// The keyword `AND`.
  KEY_AND,
  /// The keyword `OR`.
  KEY_OR,
  /// The keyword `NAND`.
  KEY_NAND,
  /// The keyword `NOR`.
  KEY_NOR,
  /// The keyword `XOR`.
  KEY_XOR,
  /// The keyword `XNOR`.
  KEY_XNOR,
  /// The keyword `MUX`.
  KEY_MUX,
  /// The keyword `REG`.
  KEY_REG,
  /// The keyword `CONCAT`.
  KEY_CONCAT,
  /// The keyword `SELECT`.
  KEY_SELECT,
  /// The keyword `SLICE`.
  KEY_SLICE,
  /// The keyword `ROM`.
  KEY_ROM,
  /// The keyword `RAM`.
  KEY_RAM,
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
  /// The byte index into the input buffer of this token.
  SourceLocation position;
};

#endif // NETLIST_TOKEN_HPP
