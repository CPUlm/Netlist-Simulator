#include <gtest/gtest.h>

#include "dependency_graph.hpp"

TEST(DependencyGraphTest, build) {
  // INPUT a, b
  // OUTPUT c
  // VAR a, b, c, d
  // c = XOR d d
  // d = AND a b

  ProgramBuilder builder;
  auto a = builder.add_register(1, {}, RIF_INPUT);
  auto b = builder.add_register(1, {}, RIF_INPUT);
  auto c = builder.add_register(1, {}, RIF_INPUT);
  auto d = builder.add_register(1, {}, RIF_INPUT);

  builder.add_xor(c, d, d);
  builder.add_and(d, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  auto dependency_graph = DependencyGraph::build(program);
  EXPECT_TRUE(dependency_graph.depends(c, d));
  EXPECT_TRUE(dependency_graph.depends(d, a));
  EXPECT_TRUE(dependency_graph.depends(d, b));
  EXPECT_FALSE(dependency_graph.depends(a, a));
  EXPECT_FALSE(dependency_graph.depends(c, b));
  EXPECT_FALSE(dependency_graph.depends(d, c));
}

TEST(DependencyGraphTest_build_Test, schedule) {
  // INPUT a, b
  // OUTPUT o, c
  // VAR
  //  _l_10_50, _l_11_49, _l_16_22, _l_17_21, _l_7_52, _l_9_51, a, b, c,
  //  c_n1_27, o, s_n_26
  // IN
  // o = AND s_n_26 s_n_26
  // o = AND s_n_26 s_n_26
  // c = OR _l_9_51 _l_11_49
  // s_n_26 = XOR _l_7_52 c_n1_27
  // _l_7_52 = XOR _l_16_22 _l_17_21
  // _l_9_51 = AND _l_16_22 _l_17_21
  // _l_10_50 = XOR _l_16_22 _l_17_21
  // _l_11_49 = AND _l_10_50 c_n1_27
  //  c_n1_27 = 0
  // _l_16_22 = SELECT 0 a
  // _l_17_21 = SELECT 0 b

  ProgramBuilder builder;
  const auto _l_10_50 = builder.add_register(1, "_l_10_50", 0);
  const auto _l_11_49 = builder.add_register(1, "_l_11_49", 0);
  const auto _l_16_22 = builder.add_register(1, "_l_16_22", 0);
  const auto _l_17_21 = builder.add_register(1, "_l_17_21", 0);
  const auto _l_7_52 = builder.add_register(1, "_l_7_52", 0);
  const auto _l_9_51 = builder.add_register(1, "_l_9_51", 0);
  const auto a = builder.add_register(1, "a", 1);
  const auto b = builder.add_register(1, "b", 1);
  const auto c = builder.add_register(1, "c", 2);
  const auto c_n1_27 = builder.add_register(1, "c_n1_27", 0);
  const auto o = builder.add_register(1, "o", 2);
  const auto s_n_26 = builder.add_register(1, "s_n_26", 0);
  builder.add_and(o, s_n_26, s_n_26);
  builder.add_and(o, s_n_26, s_n_26);
  builder.add_or(c, _l_9_51, _l_11_49);
  builder.add_xor(s_n_26, _l_7_52, c_n1_27);
  builder.add_xor(_l_7_52, _l_16_22, _l_17_21);
  builder.add_and(_l_9_51, _l_16_22, _l_17_21);
  builder.add_xor(_l_10_50, _l_16_22, _l_17_21);
  builder.add_and(_l_11_49, _l_10_50, c_n1_27);
  builder.add_const(c_n1_27, 0);
  builder.add_select(_l_16_22, 0, a);
  builder.add_select(_l_17_21, 0, b);
  auto program = builder.build();

  DependencyGraph graph = DependencyGraph::build(program);
  graph.schedule();

  ASSERT_EQ(program->registers.size(), 12);
  ASSERT_EQ(program->instructions.size(), 11);

  EXPECT_EQ(program->instructions[0]->output, _l_16_22);
  EXPECT_EQ(program->instructions[1]->output, _l_17_21);
  EXPECT_EQ(program->instructions[2]->output, _l_9_51);
  EXPECT_EQ(program->instructions[3]->output, _l_10_50);
  EXPECT_EQ(program->instructions[4]->output, c_n1_27);
  EXPECT_EQ(program->instructions[5]->output, _l_11_49);
  EXPECT_EQ(program->instructions[5]->output, c);
  EXPECT_EQ(program->instructions[5]->output, _l_7_52);
  EXPECT_EQ(program->instructions[5]->output, s_n_26);
  EXPECT_EQ(program->instructions[5]->output, o);
  EXPECT_EQ(program->instructions[5]->output, o);
}
