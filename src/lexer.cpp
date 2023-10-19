#include "lexer.hpp"

#include <cassert>
#include <algorithm>

std::string &&str_toupper(std::string &&s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::toupper(c); });
  return std::move(s);
}

void Lexer::DataBuffer::next_char() {
  if (*m_cur == '\n') {
    // We pass a new line
    m_line++;
    m_col = 0;
  }

  if (!is_eof()) { // If EOF do nothing
    m_cur++;
    m_col++;
  }
}

Lexer::Lexer(ReportContext &context, const char *input) : m_buf(input), m_context(context) {}

/// Returns true if the given ASCII character is a whitespace.
/// Our definition of whitespace is limited to ' ', '\t', '\n' and '\r'.
[[nodiscard]] static inline bool is_whitespace(char ch) {
  switch (ch) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':return true;
  default:return false;
  }
}

/// Returns true if the given ASCII character is a decimal digit.
[[nodiscard]] static inline bool is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
}

/// Returns true if the given ASCII character is a non zero decimal digit.
[[nodiscard]] static inline bool is_non_zero_digit(char ch) {
  return (ch >= '1' && ch <= '9');
}

/// Returns true if the given ASCII character is a valid first character for
/// an identifier.
[[nodiscard]] static inline bool is_start_ident(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
}

/// Returns true if the given ASCII character is a valid middle character for
/// an identifier.
[[nodiscard]] static inline bool is_cont_ident(char ch) {
  return is_start_ident(ch) || is_digit(ch) || ch == '\'';
}

void Lexer::tokenize(Token &token) {
  // We use an infinite loop here to continue lexing when encountering an
  // unknown character (which we just ignore after emitting an error) or to
  // skip comments. Each successfully token is directly returned inside the
  // loop.
  while (true) {
    skip_whitespace();

    switch (m_buf.current_char()) {
    case '\0': // End-Of-Input reached !
      token.kind = TokenKind::EOI;
      token.spelling = {};
      token.position = {m_buf.current_line(), m_buf.current_column()};
      return;

    case '=': // Equal Token Found
      token.kind = TokenKind::EQUAL;
      token.spelling = std::string_view(m_buf.current_pos(), 1);
      token.position = {m_buf.current_line(), m_buf.current_column()};
      m_buf.next_char(); // eat the character
      return;

    case ',': // Comma Token Found
      token.kind = TokenKind::COMMA;
      token.spelling = std::string_view(m_buf.current_pos(), 1);
      token.position = {m_buf.current_line(), m_buf.current_column()};
      m_buf.next_char(); // eat the character
      return;

    case ':': // Colon Token Found
      token.kind = TokenKind::COLON;
      token.spelling = std::string_view(m_buf.current_pos(), 1);
      token.position = {m_buf.current_line(), m_buf.current_column()};
      m_buf.next_char(); // eat the character
      return;

    case '#': // Comment Beginning Found
      skip_comment();
      continue; // get the next valid token

    default:
      if (is_start_ident(m_buf.current_char())) {
        tokenize_identifier(token);
        return;
      } else if (is_non_zero_digit(m_buf.current_char())) {
        tokenize_integer(token);
        return;
      }
      // Bad, we reached an unknown character.
      m_context.report(ReportSeverity::WARNING)
          .with_message("Unknown character found : '{}' (code : {:#x}).", m_buf.current_char(), m_buf.current_char())
          .with_location({m_buf.current_line(), m_buf.current_column()})
          .with_note("Ignoring character.").build().print();
    }
  }
}

void Lexer::skip_whitespace() {
  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_whitespace()
  // returns false for such a character.
  while (is_whitespace(m_buf.current_char())) {
    m_buf.next_char();
  }
}

void Lexer::skip_comment() {
  assert(m_buf.current_char() == '#');

  // Eat a whole line after '#'
  while (!m_buf.is_eof() && m_buf.current_char() != '\n') {
    m_buf.next_char();
  }
}

const std::unordered_map<std::string_view, TokenKind> spelling2keyword = {
    {"INPUT", TokenKind::KEY_INPUT},
    {"OUTPUT", TokenKind::KEY_OUTPUT},
    {"VAR", TokenKind::KEY_VAR},
    {"IN", TokenKind::KEY_IN},

    {"NOT", TokenKind::KEY_NOT},
    {"AND", TokenKind::KEY_AND},
    {"NAND", TokenKind::KEY_NAND},
    {"OR", TokenKind::KEY_OR},
    {"XOR", TokenKind::KEY_XOR},

    {"MUX", TokenKind::KEY_MUX},
    {"REG", TokenKind::KEY_REG},
    {"CONCAT", TokenKind::KEY_CONCAT},
    {"SELECT", TokenKind::KEY_SELECT},
    {"SLICE", TokenKind::KEY_SLICE},
    {"ROM", TokenKind::KEY_ROM},
    {"RAM", TokenKind::KEY_RAM},
};

void Lexer::tokenize_identifier(Token &token) {
  assert(is_start_ident(m_buf.current_char()));

  const char *begin = m_buf.current_pos();
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char(); // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_cont_ident()
  // returns false for such a character.
  while (is_cont_ident(m_buf.current_char())) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.spelling = std::string_view(begin, std::distance(begin, end));
  const std::string uppercase_keyword = str_toupper(std::move(std::string(token.spelling)));

  if (spelling2keyword.contains(uppercase_keyword)) {
    token.kind = spelling2keyword.at(token.spelling);
  } else {
    token.kind = TokenKind::IDENTIFIER;
  }

  token.position = {m_buf.current_line(), column_begin};
}

void Lexer::tokenize_integer(Token &token) {
  assert(is_non_zero_digit(m_buf.current_char()));

  const char *begin = m_buf.current_pos();
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char(); // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_digit() returns
  // false for such a character.
  while (is_digit(m_buf.current_char())) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.kind = TokenKind::INTEGER;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = {m_buf.current_line(), column_begin};
}
