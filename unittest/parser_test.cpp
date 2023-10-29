#include <gtest/gtest.h>

#include "parser.hpp"

class ParserTest : public ::testing::Test {
public:
  ReportContext ctx = ReportContext("file", false);
};

template<class T>
const T &expr_cast(const Expression::ptr &expr) {
  return *static_cast<const T *>(expr.get());
}

template<class T>
std::shared_ptr<const T> arg_cast(const Argument::ptr &arg) {
  return std::static_pointer_cast<const T>(arg);
}

TEST_F(ParserTest, trivial_program) {
  Lexer lexer(ctx, "INPUT\nOUTPUT\nVAR\nIN");
  Parser parser(ctx, lexer);
  const Program::ptr p = parser.parse_program();

  EXPECT_EQ(p->get_inputs().size(), 0); // No Input
  EXPECT_EQ(p->get_outputs().size(), 0); // No Output
  EXPECT_EQ(p->get_equations().size(), 0); // No equations
}

TEST_F(ParserTest, output_has_value_defined) {
  Lexer lexer(ctx, "INPUT x\nOUTPUT x, y\nVAR x, y\nIN y = 1");
  Parser parser(ctx, lexer);
  const Program::ptr p = parser.parse_program();

  ASSERT_EQ(p->get_inputs().size(), 1); // One Input

  const Variable::ptr &x = *p->get_inputs().begin();
  EXPECT_EQ(x->get_name(), "x");
  EXPECT_EQ(x->get_bus_size(), 1);
  EXPECT_EQ(x->get_kind(), Argument::Kind::VARIABLE);
  EXPECT_EQ(x->get_repr(), "x");

  ASSERT_EQ(p->get_equations().size(), 1); // One Equation

  const auto &eq = p->get_equations().begin();
  const Variable::ptr &y = eq->first; // The first variable of the equation
  EXPECT_EQ(y->get_name(), "y");
  EXPECT_EQ(y->get_bus_size(), 1);
  EXPECT_EQ(y->get_kind(), Argument::Kind::VARIABLE);
  EXPECT_EQ(y->get_repr(), "y");

  const Expression::ptr &expr = eq->second;
  EXPECT_EQ(expr->get_kind(), Expression::Kind::ARG);
  EXPECT_EQ(expr->get_bus_size(), 1);
  const auto &arg_expr = expr_cast<ArgExpression>(eq->second);

  const Argument::ptr &arg = arg_expr.get_argument();
  EXPECT_EQ(arg->get_kind(), Argument::Kind::CONSTANT);
  EXPECT_EQ(arg->get_bus_size(), 1);
  EXPECT_EQ(arg->get_repr(), "1");

  const Constant::ptr cst = arg_cast<Constant>(arg);
  EXPECT_EQ(cst->get_value(), 1);

  ASSERT_EQ(p->get_outputs().size(), 2); // Two Output
  // x is an output
  ASSERT_NE(std::find(p->get_outputs().begin(), p->get_outputs().end(), x), p->get_outputs().end());
  // y is an output
  ASSERT_NE(std::find(p->get_outputs().begin(), p->get_outputs().end(), y), p->get_outputs().end());

}

TEST_F(ParserTest, variable_size) {
  Lexer lexer(ctx, "INPUT x, y, z\nOUTPUT\nVAR x, y:1, z:5\nIN");
  Parser parser(ctx, lexer);
  const Program::ptr p = parser.parse_program();

  ASSERT_EQ(p->get_inputs().size(), 3); // 3 Inputs

  for (const Variable::ptr &v : p->get_inputs()) {
    if (v->get_name() == "x") {
      ASSERT_EQ(v->get_repr(), "x");
      ASSERT_EQ(v->get_bus_size(), 1);
    } else if (v->get_name() == "y") {
      ASSERT_EQ(v->get_repr(), "y");
      ASSERT_EQ(v->get_bus_size(), 1);
    } else if (v->get_name() == "z") {
      ASSERT_EQ(v->get_repr(), "z");
      ASSERT_EQ(v->get_bus_size(), 5);
    } else {
      FAIL() << "No variable other than 'x', 'y' or 'z'.";
    }
  }

  ASSERT_EQ(p->get_outputs().size(), 0); // No Output
  ASSERT_EQ(p->get_equations().size(), 0); // No Equations
}

// TODO : Add tests on all Parser's Errors
// TODO : Add tests on equations...