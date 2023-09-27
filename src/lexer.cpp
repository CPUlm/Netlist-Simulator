#include "lexer.hpp"

#include "keywords.hpp"

#include <cassert>

Lexer::Lexer(const char *input) : m_input(input), m_cursor(input) {}

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

/// Returns true if the given ASCII character is a decimal digit.
[[nodiscard]] static inline bool is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
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

    switch (*m_cursor) {
    case '\0': // End-Of-Input reached !
      token.kind = TokenKind::EOI;
      token.spelling = {};
      token.position = std::distance(m_input, m_cursor);
      return;

    case '=':
      token.kind = TokenKind::EQUAL;
      token.spelling = std::string_view(m_cursor, /* count= */ 1);
      token.position = std::distance(m_input, m_cursor);
      ++m_cursor; // eat the character
      return;

    case ',':
      token.kind = TokenKind::COMMA;
      token.spelling = std::string_view(m_cursor, /* count= */ 1);
      token.position = std::distance(m_input, m_cursor);
      ++m_cursor; // eat the character
      return;

    case ':':
      token.kind = TokenKind::COLON;
      token.spelling = std::string_view(m_cursor, /* count= */ 1);
      token.position = std::distance(m_input, m_cursor);
      ++m_cursor; // eat the character
      return;

    case '#':
      skip_comment();
      continue; // get the next valid token

    default:
      if (is_start_ident(*m_cursor)) {
        tokenize_identifier(token);
        return;
      } else if (is_digit(*m_cursor)) {
        tokenize_integer(token);
        return;
      }

      // Bad, we reached an unknown character.
      // FIXME: we should emit an error, for now we just ignore it
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
  auto *keyword_info =
      KeywordHashTable::lookup(token.spelling.data(), token.spelling.size());
  if (keyword_info != nullptr) {
    token.kind = keyword_info->token_kind;
  } else {
    token.kind = TokenKind::IDENTIFIER;
  }

  token.position = std::distance(m_input, begin);
}

void Lexer::tokenize_integer(Token &token) {
  assert(is_digit(*m_cursor));

  const char *begin = m_cursor;

  ++m_cursor; // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_digit() returns
  // false for such a character.
  while (is_digit(*m_cursor)) {
    ++m_cursor;
  }

  const char *end = m_cursor;

  token.kind = TokenKind::INTEGER;
  token.spelling = std::string_view(begin, std::distance(begin, end));
  token.position = std::distance(m_input, begin);
}
