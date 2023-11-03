#include "lexer.hpp"

#include "keywords.hpp"

#include <cassert>

Lexer::Lexer(ReportManager &report_manager, const char *input)
    : m_report_manager(report_manager), m_input(input), m_cursor(input) {
}

/// Returns true if the given ASCII character is a whitespace.
/// Our definition of whitespace is limited to ' ', '\t', '\n' and '\r'.
[[nodiscard]] static inline bool is_whitespace(char ch) {
  switch (ch) {
  case ' ':
  case '\t':
  case '\n':
  case '\r':
    return true;
  default:
    return false;
  }
}

/// Returns true if the given ASCII character is a valid binary digit.
[[nodiscard]] static inline bool is_bin_digit(char ch) {
  return ch == '0' || ch == '1';
}

/// Returns true if the given ASCII character is a valid decimal digit.
[[nodiscard]] static inline bool is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
}

/// Returns true if the given ASCII character is a valid hexadecimal digit.
[[nodiscard]] static inline bool is_hex_digit(char ch) {
  return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
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
  // skip comments. Each successful token is directly returned inside the
  // loop.
  while (true) {
    skip_whitespace();

    switch (*m_cursor) {
    case '\0': // End-Of-Input reached!
      token.kind = TokenKind::EOI;
      token.spelling = {};
      token.position = get_current_location();
      return;

    case '=':
      token.kind = TokenKind::EQUAL;
      token.spelling = std::string_view(m_cursor, /* count= */ 1);
      token.position = get_current_location();
      ++m_cursor; // eat the character
      return;

    case ',':
      token.kind = TokenKind::COMMA;
      token.spelling = std::string_view(m_cursor, /* count= */ 1);
      token.position = get_current_location();
      ++m_cursor; // eat the character
      return;

    case ':':
      token.kind = TokenKind::COLON;
      token.spelling = std::string_view(m_cursor, /* count= */ 1);
      token.position = get_current_location();
      ++m_cursor; // eat the character
      return;

    case '#':
      skip_comment();
      continue; // get the next valid token

    default: {
      if (is_start_ident(*m_cursor)) {
        tokenize_identifier(token);
        return;
      } else if (is_digit(*m_cursor)) {
        // As 0 is a valid digit, this branch also matches radix-prefixed integers.
        tokenize_integer(token);
        return;
      }

      // Bad, we reached an unknown character.
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(get_current_location())
          .with_message("unknown character found")
          .finish()
          .exit();
    }
    }
  }
}

void Lexer::skip_whitespace() {
  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_whitespace()
  // returns false for such a character.
  while (is_whitespace(*m_cursor))
    ++m_cursor;
}

void Lexer::skip_comment() {
  assert(*m_cursor == '#');

  ++m_cursor; // eat `#`

  // CR-LF line endings are also correctly recognized because of the second
  // byte LF.
  while (*m_cursor != '\0' && *m_cursor != '\n') {
    ++m_cursor;
  }
}

void Lexer::tokenize_identifier(Token &token) {
  assert(is_start_ident(*m_cursor));

  const char *begin = m_cursor;

  ++m_cursor; // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_cont_ident()
  // returns false for such a character.
  while (is_cont_ident(*m_cursor)) {
    ++m_cursor;
  }

  const char *end = m_cursor;

  token.spelling = std::string_view(begin, std::distance(begin, end));
  auto *keyword_info = KeywordHashTable::lookup(token.spelling.data(), token.spelling.size());
  if (keyword_info != nullptr) {
    token.kind = keyword_info->token_kind;
  } else {
    token.kind = TokenKind::IDENTIFIER;
  }

  token.position = SourceLocation::from_offset(std::distance(m_input, begin));
}

void Lexer::tokenize_integer(Token &token) {
  assert(is_digit(*m_cursor));

  const char *begin = m_cursor;

  unsigned radix = m_default_radix;
  bool has_explicit_radix = false;
  if (*m_cursor == '0') {
    ++m_cursor;
    switch (*m_cursor) {
    case 'b':
    case 'B':
      ++m_cursor;
      radix = 2;
      has_explicit_radix = true;
      break;
    case 'd':
    case 'D':
      ++m_cursor;
      radix = 10;
      has_explicit_radix = true;
      break;
    case 'x':
    case 'X':
      ++m_cursor;
      radix = 16;
      has_explicit_radix = true;
      break;
    }

    if (has_explicit_radix && !is_hex_digit(*m_cursor)) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(get_current_location())
          .with_span({get_current_location(), 1})
          .with_message("expected a digit after a radix prefix in a constant")
          .finish()
          .exit();
    }
  }

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_digit() returns
  // false for such a character.
  while (is_hex_digit(*m_cursor)) {
    bool invalid_digit = false;
    switch (radix) {
    case 2:
      invalid_digit = !is_bin_digit(*m_cursor);
      break;
    case 10:
      invalid_digit = !is_digit(*m_cursor);
      break;
    case 16:
      invalid_digit = !is_hex_digit(*m_cursor);
      break;
    default:
      assert(false && "unreachable");
    }

    if (invalid_digit) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(get_current_location())
          .with_span({get_current_location(), 1})
          .with_message("invalid digit in the constant")
          .with_note("the radix of the constant is {}", radix)
          .finish()
          .exit();
    }

    ++m_cursor;
  }

  const char *end = m_cursor;

  token.kind = TokenKind::INTEGER;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = SourceLocation::from_offset(std::distance(m_input, begin));
}

SourceLocation Lexer::get_current_location() const {
  return SourceLocation::from_offset(std::distance(m_input, m_cursor));
}
