#ifndef NETLIST_LEXER_HPP
#define NETLIST_LEXER_HPP

#include "identifier_table.hpp"
#include "token.hpp"

class Lexer {
public:
  explicit Lexer(IdentifierTable &identifier_table, const char *m_input);

  void tokenize(Token &token);

private:
  void skip_whitespace();
  void tokenize_identifier(Token &token);

private:
  IdentifierTable &m_identifier_table;
  const char *m_cursor = nullptr;
};

#endif // NETLIST_LEXER_HPP
