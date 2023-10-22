#include "lexer.hpp"

#include <algorithm>
#include <cassert>

std::string &&str_toupper(std::string &&s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return std::toupper(c); });
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

Lexer::Lexer(ReportContext &context, const char *input)
    : m_buf(input), m_context(context) {}

/// Returns true if the given ASCII character is a whitespace,
/// ie. ' ', '\t', '\n' or '\r'.
bool Lexer::is_whitespace(const Lexer::DataBuffer &b) {
  return b.current_char() == ' '
      || b.current_char() == '\t'
      || b.current_char() == '\n'
      || b.current_char() == '\r';
}

///// Returns true if the given ASCII character is a binary digit.
bool Lexer::is_binary_digit(const Lexer::DataBuffer &b) {
  return b.current_char() == '0' || b.current_char() == '1';
}

///// Returns true if the given ASCII character is a decimal digit.
bool Lexer::is_decimal_digit(const Lexer::DataBuffer &b) {
  return b.current_char() >= '0' && b.current_char() <= '9';
}

///// Returns true if the given ASCII character is a hexadecimal digit.
bool Lexer::is_hexadecimal_digit(const Lexer::DataBuffer &b) {
  return (b.current_char() >= '0' && b.current_char() <= '9')
      || (b.current_char() >= 'a' && b.current_char() <= 'f')
      || (b.current_char() >= 'A' && b.current_char() <= 'F');
}

/// Returns true if the given ASCII character is a non zero decimal digit.
bool Lexer::is_non_zero_decimal_digit(const Lexer::DataBuffer &b) {
  return b.current_char() >= '1' && b.current_char() <= '9';
}

/// Returns true if the given ASCII character is a valid first character for
/// an identifier.
bool Lexer::is_ident_start(const Lexer::DataBuffer &b) {
  return (b.current_char() >= 'a' && b.current_char() <= 'z')
      || (b.current_char() >= 'A' && b.current_char() <= 'Z')
      || (b.current_char() == '_');
}

/// Returns true if the given ASCII character is a valid middle character for
/// an identifier.
bool Lexer::is_ident_body(const Lexer::DataBuffer &b) {
  return is_ident_start(b) || is_decimal_digit(b) || (b.current_char() == '\'');
}

void Lexer::tokenize(Token &token) {
  // We use an infinite loop here to continue lexing when encountering an
  // unknown character (which we just ignore after emitting an error) or to
  // skip comments. Each successfully token is directly returned inside the
  // loop.
  for (;;) {
    skip_whitespace();

    switch (m_buf.current_char()) {
    case '\0':
      token.kind = TokenKind::EOI;
      token.spelling = {};
      token.position = {m_buf.current_line(), m_buf.current_column()};
      return;

    case '=':
      token.kind = TokenKind::EQUAL;
      token.spelling = std::string_view(m_buf.current_pos(), 1);
      token.position = {m_buf.current_line(), m_buf.current_column()};
      m_buf.next_char(); // eat the character
      return;

    case ',':
      token.kind = TokenKind::COMMA;
      token.spelling = std::string_view(m_buf.current_pos(), 1);
      token.position = {m_buf.current_line(), m_buf.current_column()};
      m_buf.next_char(); // eat the character
      return;

    case ':':
      token.kind = TokenKind::COLON;
      token.spelling = std::string_view(m_buf.current_pos(), 1);
      token.position = {m_buf.current_line(), m_buf.current_column()};
      m_buf.next_char(); // eat the character
      return;

    case '#':
      skip_comment();
      continue; // get the next valid token

    case '0': {
      DataBuffer buf = m_buf;
      buf.next_char();
      // Can either be BINARY_CONSTANT or DECIMAL_CONSTANT or HEXADECIMAL_CONSTANT
      if (buf.current_char() == 'd') {
        // It's a DECIMAL_CONSTANT
        tokenize_decimal_constant(token);
        return;
      } else if (buf.current_char() == 'x') {
        // It's a HEXADECIMAL_CONSTANT
        tokenize_hexadecimal_constant(token);
        return;
      } else {
        // Can only be a BINARY_CONSTANT
        tokenize_binary_constant(token);
        return;
      }
    }
    case '1': {
      // Can be a BINARY_CONSTANT or an INTEGER
      // We return the longest token possible.
      size_t int_length = parse_max_int_length(m_buf);
      size_t bin_length = parse_max_bin_length(m_buf);

      if (int_length > bin_length) {
        tokenize_integer(token);
        return;
      } else {
        tokenize_binary_constant(token);
        return;
      }
    }
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      tokenize_integer(token);
      return;

    default:
      if (is_ident_start(m_buf)) {
        tokenize_identifier(token);
        return;
      }
      // Bad, we reached an unknown character.
      m_context.report(ReportSeverity::ERROR)
          .with_message("Unknown character found : '{}' (code : {:#x}).",
                        m_buf.current_char(), m_buf.current_char())
          .with_location({m_buf.current_line(), m_buf.current_column()})
          .build()
          .exit();
    }
  }
}

void Lexer::skip_whitespace() {
  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_whitespace()
  // returns false for such a character.
  while (is_whitespace(m_buf)) {
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

size_t Lexer::parse_max_int_length(Lexer::DataBuffer b) {
  size_t rep = 0;
  while (is_decimal_digit(b)) {
    rep++;
    b.next_char();
  }

  return rep;
}

size_t Lexer::parse_max_bin_length(Lexer::DataBuffer b) {
  size_t rep = 0;
  while (is_binary_digit(b)) {
    rep++;
    b.next_char();
  }

  return rep;
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
  assert(is_ident_start(m_buf));

  const char *begin = m_buf.current_pos();
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char(); // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_cont_ident()
  // returns false for such a character.
  while (is_ident_body(m_buf)) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.spelling = std::string_view(begin, std::distance(begin, end));
  const std::string uppercase_keyword =
      str_toupper(std::move(std::string(token.spelling)));

  if (spelling2keyword.contains(uppercase_keyword)) {
    token.kind = spelling2keyword.at(token.spelling);
  } else {
    token.kind = TokenKind::IDENTIFIER;
  }

  token.position = {m_buf.current_line(), column_begin};
}

void Lexer::tokenize_integer(Token &token) {
  assert(is_non_zero_decimal_digit(m_buf));

  const char *begin = m_buf.current_pos();
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char(); // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_digit() returns
  // false for such a character.
  while (is_decimal_digit(m_buf)) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.kind = TokenKind::INTEGER;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = {m_buf.current_line(), column_begin};
}

void Lexer::tokenize_decimal_constant(Token &token) {
  assert(m_buf.current_char() == '0');
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char();
  assert(m_buf.current_char() == 'd');
  m_buf.next_char();

  const char *begin = m_buf.current_pos();

  while (is_decimal_digit(m_buf)) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.kind = TokenKind::DECIMAL_CONSTANT;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = {m_buf.current_line(), column_begin};
}

void Lexer::tokenize_hexadecimal_constant(Token &token) {
  assert(m_buf.current_char() == '0');
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char();
  assert(m_buf.current_char() == 'x');
  m_buf.next_char();

  const char *begin = m_buf.current_pos();

  while (is_hexadecimal_digit(m_buf)) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.kind = TokenKind::HEXADECIMAL_CONSTANT;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = {m_buf.current_line(), column_begin};
}

void Lexer::tokenize_binary_constant(Token &token) {
  assert(is_binary_digit(m_buf));

  const char *begin = m_buf.current_pos();
  const uint32_t column_begin = m_buf.current_column();

  m_buf.next_char();

  while (is_binary_digit(m_buf)) {
    m_buf.next_char();
  }

  const char *end = m_buf.current_pos();

  token.kind = TokenKind::BINARY_CONSTANT;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = {m_buf.current_line(), column_begin};
}

