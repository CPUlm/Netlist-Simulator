#include "lexer.hpp"

Lexer::Lexer(IdentifierTable &identifier_table, const char *m_input)
    : m_identifier_table(identifier_table), m_cursor(m_input) {}

/// Returns true if the given ASCII character is a valid first character for
/// an identifier.
[[nodiscard]] bool is_start_ident(char ch) {
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');
}

/// Returns true if the given ASCII character is a valid middle character for
/// an identifier.
[[nodiscard]] bool is_cont_ident(char ch) {
  return is_start_ident(ch) || (ch >= '0' && ch <= '9');
}

void Lexer::tokenize(Token &token) {
  skip_whitespace();

  // We use an infinite loop here to continue lexing when encountering an
  // unknown character (which we just ignore after emitting an error). Each
  // successfully token is directly returned inside the loop.
  while (true) {
    switch (*m_cursor) {
    case '\0': // End-Of-Input reached !
      token.kind = TokenKind::EOI;
      return;

    case '=':
      token.kind = TokenKind::EQUAL;
      return;

    case ',':
      token.kind = TokenKind::COMMA;
      return;

    default:
      if (is_start_ident(*m_cursor)) {
        tokenize_identifier(token);
        return;
      }

      // Bad, we reached an unknown character.
      // FIXME: we should emit an error, for now we just ignore it
    }
  }
}

/// Returns true if the given ASCII character is a whitespace.
/// Our definition of whitespace is limited to ' ', '\t', '\n' and '\r'.
[[nodiscard]] static bool is_whitespace(char ch) {
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

void Lexer::skip_whitespace() {
  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_whitespace()
  // returns false for such a character.
  while (is_whitespace(*m_cursor))
    ++m_cursor;
}

void Lexer::tokenize_identifier(Token &token) {
  const char *begin = m_cursor;

  ++m_cursor; // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_cont_ident()
  // returns false for such a character.
  while (is_cont_ident(*m_cursor)) {
    ++m_cursor;
  }

  const char *end = m_cursor;

  auto spelling = std::string_view(begin, std::distance(begin, end));
  auto& identifier_info = m_identifier_table.get(spelling);
  token.kind = identifier_info.get_token_kind();
}
