#include <gtest/gtest.h>

#include "lexer.hpp"

class LexerTest : public ::testing::Test {
public:
  IdentifierTable identifier_table;

  void SetUp() override { identifier_table.register_keywords(); }
};

TEST_F(LexerTest, punctuation) {
  Lexer lexer(identifier_table, "= ,");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EQUAL);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::COMMA);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
}

TEST_F(LexerTest, identifiers) {
  Lexer lexer(identifier_table, "a c_out _l_2");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
}

TEST_F(LexerTest, keywords) {
  Lexer lexer(identifier_table, "OUTPUT VAR");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_OUTPUT);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_VAR);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
}
