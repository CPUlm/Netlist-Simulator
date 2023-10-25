#ifndef NETLIST_LEXER_HPP
#define NETLIST_LEXER_HPP

#include "report.hpp"
#include "token.hpp"

/// The lexical analyser for the netlist language.
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
  /// Just skip eagerly any whitespace found.
  void skip_whitespace();

  /// Just skip until the end of the line. This function should only be
  /// called when the lexer is located at the start of a comment.
  void skip_comment();

  /// Tokenizes an IDENTIFIER. This function should only be called when the
  /// lexer is located at the first valid character of an identifier.
  void tokenize_identifier(Token &token);

  /// Tokenizes a BUS_SIZE. This function should only be called when the
  /// lexer is located at the first valid character of an integer ie. a
  /// decimal digits
  void tokenize_integer(Token &token);

  /// Tokenizes a BINARY_CONSTANT. This function should only be called when the
  /// lexer is located at the first valid characters of a binary constant ie. '0b'
  void tokenize_binary_constant(Token &Token);

  /// Tokenizes a DECIMAL_CONSTANT. This function should only be called when the
  /// lexer is located at the first valid characters of a decimal constant ie. '0d'
  void tokenize_decimal_constant(Token &Token);

  /// Tokenizes a HEXADECIMAL_CONSTANT. This function should only be called when the
  /// lexer is located at the first valid characters of a hexadecimal constant ie. '0x'
  void tokenize_hexadecimal_constant(Token &Token);
private:
  class DataBuffer {
  public:
    explicit DataBuffer(const char *beginning) noexcept
        : m_line(0), m_col(0), m_cur(beginning) {}

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
  } m_buf;

  ReportContext &m_context;

  [[nodiscard]] static inline bool is_whitespace(const DataBuffer &b);
  [[nodiscard]] static inline bool is_binary_digit(const DataBuffer &b);
  [[nodiscard]] static inline bool is_decimal_digit(const DataBuffer &b);
  [[nodiscard]] static inline bool is_hexadecimal_digit(const DataBuffer &b);
  [[nodiscard]] static inline bool is_ident_start(const Lexer::DataBuffer &b);
  [[nodiscard]] static inline bool is_ident_body(const Lexer::DataBuffer &b);
};

#endif // NETLIST_LEXER_HPP
