#include "parser.hpp"

const std::unordered_map<TokenKind, std::string> token_spelling = {
    {TokenKind::EOI, "End-Of-File"},
    {TokenKind::IDENTIFIER, "Identifier (such as '_l_10')"},
    {TokenKind::INTEGER, "Integer (such as '42')"},

    {TokenKind::EQUAL, "="},
    {TokenKind::COMMA, ","},
    {TokenKind::COLON, ":"},

    {TokenKind::KEY_INPUT, "INPUT"},
    {TokenKind::KEY_OUTPUT, "OUTPUT"},
    {TokenKind::KEY_VAR, "VAR"},
    {TokenKind::KEY_IN, "IN"},

    {TokenKind::KEY_NOT, "NOT"},
    {TokenKind::KEY_AND, "AND"},
    {TokenKind::KEY_NAND, "NAND"},
    {TokenKind::KEY_OR, "OR"},
    {TokenKind::KEY_XOR, "XOR"},

    {TokenKind::KEY_MUX, "MUX"},
    {TokenKind::KEY_REG, "REG"},
    {TokenKind::KEY_CONCAT, "CONCAT"},
    {TokenKind::KEY_SELECT, "SELECT"},
    {TokenKind::KEY_SLICE, "SLICE"},
    {TokenKind::KEY_ROM, "ROM"},
    {TokenKind::KEY_RAM, "RAM"},
};

Parser::Parser(ReportContext &context, Lexer &lexer) : m_context(context), m_lexer(lexer) {
  // Gets the first token
  m_lexer.tokenize(m_token);
}

void Parser::consume() { m_lexer.tokenize(m_token); }

void Parser::assert(const std::set<TokenKind> &token_set) const noexcept {
  if (!token_set.contains(m_token.kind)) {
    std::string tokens = token_spelling.at(*token_set.begin());

    if (token_set.size() > 1) {
      for (auto it = ++token_set.begin(); it != token_set.end(); it++) {
        tokens.append("' or '" + token_spelling.at(*it));
      }
    }

    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Unexpected token. Found : '{}', expected : '{}'", m_token.spelling, tokens)
        .build()
        .exit();
  }
}

void Parser::assert(TokenKind token) const noexcept {
  if (m_token.kind != token) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Unexpected token. Found : '{}', expected : '{}'", m_token.spelling, token_spelling.at(token))
        .build()
        .exit();
  }
}

std::unique_ptr<Program> Parser::parse_program() {
  std::unique_ptr<Program> p(new Program());

  auto inputs_vars = parse_inputs();
  auto output_vars = parse_outputs();
  auto var_decls = parse_var_decl();
  // m_token.kind = TokenKind::KEY_IN
  
  return p;
}

std::vector<Parser::VariableReference> Parser::parse_inputs() noexcept {
  assert(TokenKind::KEY_INPUT);
  consume(); // eat 'INPUT'

  std::vector<Parser::VariableReference> var;

  while (m_token.kind != TokenKind::KEY_OUTPUT) {
    assert(TokenKind::IDENTIFIER);
    var.emplace_back(m_token.spelling, m_token.position);
    consume(); // eat 'IDENTIFIER'

    assert({TokenKind::COMMA, TokenKind::KEY_OUTPUT});

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat 'COMMA'
    }
  }

  return var;
}

std::vector<Parser::VariableReference> Parser::parse_outputs() noexcept {
  assert(TokenKind::KEY_OUTPUT);
  consume(); // eat 'OUTPUT'

  std::vector<Parser::VariableReference> var;

  while (m_token.kind != TokenKind::KEY_VAR) {
    assert(TokenKind::IDENTIFIER);
    var.emplace_back(m_token.spelling, m_token.position);
    consume(); // eat 'IDENTIFIER'

    assert({TokenKind::COMMA, TokenKind::KEY_VAR});

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat 'COMMA'
    }
  }

  return var;
}

std::vector<Parser::VariableDeclaration> Parser::parse_var_decl() noexcept {
  assert(TokenKind::KEY_VAR);
  consume(); // eat 'VAR'

  std::vector<Parser::VariableDeclaration> var;

  while (m_token.kind != TokenKind::KEY_IN) {
    assert(TokenKind::IDENTIFIER);

    VariableDeclaration v = {};
    v.spelling = m_token.spelling;
    v.position = m_token.position;
    consume(); // eat 'IDENTIFIER'

    if (m_token.kind == TokenKind::COLON) {
      consume(); // eat ':'
      assert(TokenKind::INTEGER);
      auto i = parse_integer<bus_size_t>();
      if (i > max_bus_size) {
        m_context.report(ReportSeverity::ERROR)
            .with_location(m_token.position)
            .with_message("Integer '{}' is too big to be a bus size. Max bus size authorised : '{}'", i, max_bus_size)
            .build()
            .exit();
      }
      v.size = i;
    } else {
      v.size = 1;
    }
    var.push_back(v);

    assert({TokenKind::COMMA, TokenKind::KEY_IN});

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat 'COMMA'
    }
  }

  return var;
}
