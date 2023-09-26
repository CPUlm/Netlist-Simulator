#include <gtest/gtest.h>

#include "identifier_table.hpp"

TEST(IdentifierTableTest, get_not_exist) {
  IdentifierTable table;

  IdentifierInfo &foo = table.get("foo");
  EXPECT_EQ(foo.get_token_kind(), TokenKind::IDENTIFIER);
}

TEST(IdentifierTableTest, get) {
  IdentifierTable table;

  IdentifierInfo &foo = table.get("foo");
  EXPECT_EQ(foo.get_token_kind(), TokenKind::IDENTIFIER);

  foo.set_token_kind(TokenKind::KEY_OUTPUT);
  EXPECT_EQ(foo.get_token_kind(), TokenKind::KEY_OUTPUT);

  IdentifierInfo &foo_bis = table.get("foo");
  EXPECT_EQ(foo_bis.get_token_kind(), TokenKind::KEY_OUTPUT);
  EXPECT_EQ(&foo, &foo_bis); // same memory address
}

TEST(IdentifierTableTest, keywords) {
  IdentifierTable table;

  IdentifierInfo &output = table.get("OUTPUT");
  EXPECT_EQ(output.get_token_kind(), TokenKind::IDENTIFIER);

  table.register_keywords();

  EXPECT_EQ(output.get_token_kind(), TokenKind::KEY_OUTPUT);
}
