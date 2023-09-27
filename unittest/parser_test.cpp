#include <gtest/gtest.h>

#include "parser.hpp"

TEST(ParserTest, basic_program) {
  Lexer lexer("INPUT a\nOUTPUT b\nVAR a, b\nIN\nb=NOT a");
  Parser parser(lexer);
  Program program = parser.parse_program();

  EXPECT_EQ(program.get_input_names(), std::vector({std::string_view("a")}));
  EXPECT_EQ(program.get_output_names(), std::vector({std::string_view("b")}));

  const auto &values = program.get_values();
  ASSERT_EQ(values.size(), 2);
  EXPECT_EQ(values[0]->get_kind(), Value::Kind::INPUT);
  EXPECT_EQ(values[0]->get_name(), "a");
  EXPECT_EQ(values[1]->get_kind(), Value::Kind::EQUATION);
  EXPECT_EQ(values[1]->get_name(), "b");

  const auto &equations = program.get_equations();
  ASSERT_EQ(equations.size(), 1);

  const auto *equation = equations[0];
  EXPECT_EQ(equation, values[1].get());
  EXPECT_EQ(equation->is_output(), true);

  const auto *expr = equation->get_expression();
  EXPECT_EQ(expr->get_kind(), Expression::Kind::NOT);
  EXPECT_EQ(static_cast<const NotExpression *>(expr)->get_value(),
            values[0].get());
}

TEST(ParserTest, multiple_inputs) {
  Lexer lexer("INPUT a, b, c\nOUTPUT z\nVAR a, b, c, z\nIN\nz=NOT a");
  Parser parser(lexer);
  Program program = parser.parse_program();

  EXPECT_EQ(program.get_input_names(),
            std::vector({std::string_view("a"), std::string_view("b"),
                         std::string_view("c")}));
}

TEST(ParserTest, multiple_outputs) {
  Lexer lexer("INPUT a\nOUTPUT b, c, d\nVAR a, b, c, d\nIN\nb=NOT a");
  Parser parser(lexer);
  Program program = parser.parse_program();

  EXPECT_EQ(program.get_output_names(),
            std::vector({std::string_view("b"), std::string_view("c"),
                         std::string_view("d")}));
}

TEST(ParserTest, not_expr) {
  Lexer lexer("INPUT a\nOUTPUT out\nVAR a, out\nIN\nout=NOT a");
  Parser parser(lexer);
  Program program = parser.parse_program();

  const auto &values = program.get_values();
  ASSERT_EQ(values.size(), 2);
  const auto *a = values[0].get();

  const auto &equations = program.get_equations();
  ASSERT_EQ(equations.size(), 1);

  const auto *equation = equations[0];
  const auto *expr = equation->get_expression();
  EXPECT_EQ(equation->get_name(), "out");
  EXPECT_EQ(expr->get_kind(), Expression::Kind::NOT);
  EXPECT_EQ(static_cast<const NotExpression *>(expr)->get_value(), a);
}

void binary_expr_test(BinaryOp binop, const char *binop_spelling) {
  const auto input = std::string("INPUT a, b\nOUTPUT out\nVAR a, b, out\nIN\nout=") + binop_spelling + " a b";
  Lexer lexer(input.c_str());
  Parser parser(lexer);
  Program program = parser.parse_program();

  const auto &values = program.get_values();
  ASSERT_EQ(values.size(), 3);
  const auto *a = values[0].get();
  const auto *b = values[1].get();

  const auto &equations = program.get_equations();
  ASSERT_EQ(equations.size(), 1);

  const auto *equation = equations[0];
  const auto *expr = equation->get_expression();
  EXPECT_EQ(equation->get_name(), "out");
  EXPECT_EQ(expr->get_kind(), Expression::Kind::BINARY);
  EXPECT_EQ(static_cast<const BinaryExpression *>(expr)->get_operator(), binop);
  EXPECT_EQ(static_cast<const BinaryExpression *>(expr)->get_lhs(), a);
  EXPECT_EQ(static_cast<const BinaryExpression *>(expr)->get_rhs(), b);
}

TEST(ParserTest, binary_expr) {
  binary_expr_test(BinaryOp::AND, "AND");
  binary_expr_test(BinaryOp::OR, "OR");
  binary_expr_test(BinaryOp::NAND, "NAND");
  binary_expr_test(BinaryOp::XOR, "XOR");
}
