#include <gtest/gtest.h>

#include "lexer.hpp"

TEST(LexerTest, punctuation) {
  ReportManager report_manager;
  Lexer lexer(report_manager, "= , :");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EQUAL);
  EXPECT_EQ(token.spelling, "=");
  EXPECT_EQ(token.position.offset, 0);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::COMMA);
  EXPECT_EQ(token.spelling, ",");
  EXPECT_EQ(token.position.offset, 2);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::COLON);
  EXPECT_EQ(token.spelling, ":");
  EXPECT_EQ(token.position.offset, 4);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.offset, 5);
}

TEST(LexerTest, identifiers) {
  ReportManager report_manager;
  Lexer lexer(report_manager, "a c_out _l_2");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "a");
  EXPECT_EQ(token.position.offset, 0);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "c_out");
  EXPECT_EQ(token.position.offset, 2);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "_l_2");
  EXPECT_EQ(token.position.offset, 8);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.offset, 12);
}

TEST(LexerTest, keywords) {
  ReportManager report_manager;
  Lexer lexer(report_manager, "OUTPUT VAR");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_OUTPUT);
  EXPECT_EQ(token.spelling, "OUTPUT");
  EXPECT_EQ(token.position.offset, 0);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_VAR);
  EXPECT_EQ(token.spelling, "VAR");
  EXPECT_EQ(token.position.offset, 7);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.offset, 10);
}

TEST(LexerTest, integers) {
  ReportManager report_manager;
  Lexer lexer(report_manager, "0 42 0b1101 0xff 0d42");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "0");
  EXPECT_EQ(token.position.offset, 0);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "42");
  EXPECT_EQ(token.position.offset, 2);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "0b1101");
  EXPECT_EQ(token.position.offset, 5);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "0xff");
  EXPECT_EQ(token.position.offset, 12);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "0d42");
  EXPECT_EQ(token.position.offset, 17);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.offset, 21);
}

TEST(LexerTest, comments) {
  ReportManager report_manager;
  Lexer lexer(report_manager, "# =\n=\n# EOI");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EQUAL);
  EXPECT_EQ(token.spelling, "=");
  EXPECT_EQ(token.position.offset, 4);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.offset, 11);
}
