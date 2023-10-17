#include "lexer.hpp"

#include "keywords.hpp"

#include <cassert>

void DataBuffer::next_char() {
  if (*current == '\n') {
    // We pass a new line
    line++;
    col = 0;
  }

  if (!is_eof()) { // If EOF do nothing
    current++;
    col++;
  }
}

Lexer::Lexer(const char *input) : buf(input) {}

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

    switch (buf.current_char()) {
    case '\0': // End-Of-Input reached !
      token.kind = TokenKind::EOI;
      token.spelling = {};
      token.position = {buf.current_line(), buf.current_column(), 0};
      return;

    case '=': // Equal Token Found
      token.kind = TokenKind::EQUAL;
      token.spelling = std::string_view(buf.current_pos(), /* count= */ 1);
      token.position = {buf.current_line(), buf.current_column(), 1};
      buf.next_char(); // eat the character
      return;

    case ',': // Comma Token Found
      token.kind = TokenKind::COMMA;
      token.spelling = std::string_view(buf.current_pos(), /* count= */ 1);
      token.position = {buf.current_line(), buf.current_column(), 1};
      buf.next_char(); // eat the character
      return;

    case ':': // Colon Token Found
      token.kind = TokenKind::COLON;
      token.spelling = std::string_view(buf.current_pos(), /* count= */ 1);
      token.position = {buf.current_line(), buf.current_column(), 1};
      buf.next_char(); // eat the character
      return;

    case '#': // Comment Beginning Found
      skip_comment();
      continue; // get the next valid token

    default:
      if (is_start_ident(buf.current_char())) {
        tokenize_identifier(token);
        return;
      } else if (is_digit(buf.current_char())) {
        tokenize_integer(token);
        return;
      }

      // Bad, we reached an unknown character.
      // TODO : Emmit big error
    }
  }
}

void Lexer::skip_whitespace() {
  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_whitespace()
  // returns false for such a character.
  while (is_whitespace(buf.current_char())) {
    buf.next_char();
  }
}

void Lexer::skip_comment() {
  assert(buf.current_char() == '#');

  // Eat a whole line after '#'
  while (!buf.is_eof() && buf.current_char() != '\n') {
    buf.next_char();
  }
}

void Lexer::tokenize_identifier(Token &token) {
  assert(is_start_ident(buf.current_char()));

  const char *begin = buf.current_pos();
  const uint32_t column_begin = buf.current_column();

  buf.next_char(); // eat the first character

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_cont_ident()
  // returns false for such a character.
  while (is_cont_ident(buf.current_char())) {
    buf.next_char();
  }

  const char *end = buf.current_pos();
  const uint32_t token_length = std::distance(begin, end);

  token.spelling = std::string_view(begin, token_length);
  auto *keyword_info =
      KeywordHashTable::lookup(token.spelling.data(), token.spelling.size());
  // TODO : Use a regular HashTable (And so dropping gperf)
  if (keyword_info != nullptr) {
    token.kind = keyword_info->token_kind;
  } else {
    token.kind = TokenKind::IDENTIFIER;
  }

  token.position = {buf.current_line(), column_begin, token_length};
}

void Lexer::tokenize_integer(Token &token) {
  assert(is_digit(buf.current_char()));

  const char *begin = buf.current_pos();
  const uint32_t column_begin = buf.current_column();

  // This never read past the end of the input because each input buffer is
  // guaranteed to be terminated by the NUL character and is_digit() returns
  // false for such a character.
  while (is_digit(buf.current_char())) {
    buf.next_char();
  }

  const char *end = buf.current_pos();
  const uint32_t token_length = std::distance(begin, end);

  token.kind = TokenKind::INTEGER;
  token.spelling = std::string_view(begin, token_length);
  token.position = {buf.current_line(), column_begin, token_length};
}
