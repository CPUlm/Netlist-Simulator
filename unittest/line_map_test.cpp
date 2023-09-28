#include "line_map.hpp"

#include "gtest/gtest.h"

TEST(LineMapTest, empty)
{
  LineMap lm;

  // Lines and columns are 1-numbered
  EXPECT_EQ(lm.get_line_number(0), 1);
  EXPECT_EQ(lm.get_column_number(0), 1);

  // No line terminator was registered
  EXPECT_EQ(lm.get_line_number(15861), 1);
  EXPECT_EQ(lm.get_column_number(15861), 15862);
}

TEST(LineMapTest, add_newline)
{
  LineMap lm;

  lm.add_newline(50);

  // Check the position before the new line
  EXPECT_EQ(lm.get_line_number(49), 1);
  EXPECT_EQ(lm.get_column_number(49), 50);

  // Check the position of the new line
  EXPECT_EQ(lm.get_line_number(50), 2);
  EXPECT_EQ(lm.get_column_number(50), 1);

  // Check the position after the new line
  EXPECT_EQ(lm.get_line_number(51), 2);
  EXPECT_EQ(lm.get_column_number(1), 2);

  lm.add_newline(100);

  // Check the position before the old line
  EXPECT_EQ(lm.get_line_number(49), 1);
  EXPECT_EQ(lm.get_column_number(49), 50);

  // Check the position of the old line
  EXPECT_EQ(lm.get_line_number(50), 2);
  EXPECT_EQ(lm.get_column_number(50), 1);

  // Check the position after the old line
  EXPECT_EQ(lm.get_line_number(51), 2);
  EXPECT_EQ(lm.get_column_number(51), 2);
}

TEST(LineMapTest, many_adds)
{
  LineMap lm;

  for (int i = 0; i < 1000; ++i) {
    lm.add_newline(i);
  }
}

TEST(LineMapTest, get_line_start_position)
{
  LineMap lm;

  lm.add_newline(50);
  lm.add_newline(100);

  EXPECT_EQ(lm.get_line_start_position(1), 0);
  EXPECT_EQ(lm.get_line_start_position(2), 50);
  EXPECT_EQ(lm.get_line_start_position(3), 100);
}

TEST(LineMapTest, prefill)
{
  LineMap lm;

  lm.prefill("foo\nbar\r\nhello world");

  EXPECT_EQ(lm.get_line_number(2), 1);
  EXPECT_EQ(lm.get_line_number(6), 2);
  EXPECT_EQ(lm.get_line_number(11), 3);
}
