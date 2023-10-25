#include <gtest/gtest.h>

#include "lexer.hpp"

class LexerTest : public ::testing::Test {
public:
  ReportContext ctx = ReportContext("", false);
};

TEST_F(LexerTest, punctuation) {
  Lexer lexer(ctx, "= , :");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EQUAL);
  EXPECT_EQ(token.spelling, "=");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::COMMA);
  EXPECT_EQ(token.spelling, ",");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 3);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::COLON);
  EXPECT_EQ(token.spelling, ":");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 5);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 6);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 6);
}

TEST_F(LexerTest, identifiers) {
  Lexer lexer(ctx, "a c_out _l_2\n\t\rhello_world");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "a");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "c_out");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 3);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "_l_2");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 9);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "hello_world");
  EXPECT_EQ(token.position.line, 2);
  EXPECT_EQ(token.position.begin, 3);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 2);
  EXPECT_EQ(token.position.begin, 14);
}

TEST_F(LexerTest, program_keywords) {
  Lexer lexer(ctx, "OUTPUT \r INPUT\rVAR IN");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_OUTPUT);
  EXPECT_EQ(token.spelling, "OUTPUT");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_INPUT);
  EXPECT_EQ(token.spelling, "INPUT");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 10);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_VAR);
  EXPECT_EQ(token.spelling, "VAR");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 16);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_IN);
  EXPECT_EQ(token.spelling, "IN");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 20);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 22);
}

TEST_F(LexerTest, expression_keywords) {
  Lexer lexer(ctx, "NOT AND NAND OR XOR MUX REG CONCAT SELECT SLICE ROM RAM ");
  Token token;
  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_NOT);
  EXPECT_EQ(token.spelling, "NOT");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_AND);
  EXPECT_EQ(token.spelling, "AND");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 5);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_NAND);
  EXPECT_EQ(token.spelling, "NAND");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 9);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_OR);
  EXPECT_EQ(token.spelling, "OR");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 14);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_XOR);
  EXPECT_EQ(token.spelling, "XOR");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 17);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_MUX);
  EXPECT_EQ(token.spelling, "MUX");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 21);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_REG);
  EXPECT_EQ(token.spelling, "REG");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 25);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_CONCAT);
  EXPECT_EQ(token.spelling, "CONCAT");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 29);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_SELECT);
  EXPECT_EQ(token.spelling, "SELECT");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 36);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_SLICE);
  EXPECT_EQ(token.spelling, "SLICE");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 43);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_ROM);
  EXPECT_EQ(token.spelling, "ROM");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 49);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::KEY_RAM);
  EXPECT_EQ(token.spelling, "RAM");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 53);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 57);
}

TEST_F(LexerTest, basic_integers) {
  Lexer lexer(ctx, "0 42");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "0");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "42");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 3);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 5);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 5);
}

TEST_F(LexerTest, custom_integers) {
  Lexer lexer(ctx, "0b101010 101010 0x2a 0d8o 0d42");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::BINARY_CONSTANT);
  EXPECT_EQ(token.spelling, "101010");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::INTEGER);
  EXPECT_EQ(token.spelling, "101010");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 10);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::HEXADECIMAL_CONSTANT);
  EXPECT_EQ(token.spelling, "2a");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 17);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::DECIMAL_CONSTANT);
  EXPECT_EQ(token.spelling, "8");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 22);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "o");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 25);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::DECIMAL_CONSTANT);
  EXPECT_EQ(token.spelling, "42");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 27);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 31);
}

TEST_F(LexerTest, comments) {
  Lexer lexer(ctx, "# =\n=\n# EOI");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EQUAL);
  EXPECT_EQ(token.spelling, "=");
  EXPECT_EQ(token.position.line, 2);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 3);
  EXPECT_EQ(token.position.begin, 6);
}

TEST_F(LexerTest, new_lines) {
  Lexer lexer(ctx, "a\n\nb\n\n\nc\n\t\t\t\td");
  Token token;

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "a");
  EXPECT_EQ(token.position.line, 1);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "b");
  EXPECT_EQ(token.position.line, 3);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "c");
  EXPECT_EQ(token.position.line, 6);
  EXPECT_EQ(token.position.begin, 1);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::IDENTIFIER);
  EXPECT_EQ(token.spelling, "d");
  EXPECT_EQ(token.position.line, 7);
  EXPECT_EQ(token.position.begin, 5);

  lexer.tokenize(token);
  EXPECT_EQ(token.kind, TokenKind::EOI);
  EXPECT_EQ(token.spelling, "");
  EXPECT_EQ(token.position.line, 7);
  EXPECT_EQ(token.position.begin, 6);
}
