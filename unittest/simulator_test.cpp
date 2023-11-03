#include <gtest/gtest.h>

#include "simulator/simulator.hpp"

TEST(SimulatorTest, load_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_OUTPUT);
  builder.add_load(b, a);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b10011101);
}

TEST(SimulatorTest, const_expr) {
  ProgramBuilder builder;
  auto b = builder.add_register(8, "b", RIF_OUTPUT);
  builder.add_const(b, 0b10011101);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(b), 0b10011101);
}

TEST(SimulatorTest, not_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_OUTPUT);
  builder.add_not(b, a);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01100010);
}

TEST(SimulatorTest, and_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(8, "c", RIF_OUTPUT);
  builder.add_and(c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0b00001101);
}

TEST(SimulatorTest, nand_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(8, "c", RIF_OUTPUT);
  builder.add_nand(c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0b11110010);
}

TEST(SimulatorTest, or_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(8, "c", RIF_OUTPUT);
  builder.add_or(c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0b11111101);
}

TEST(SimulatorTest, nor_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(8, "c", RIF_OUTPUT);
  builder.add_nor(c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0b00000010);
}

TEST(SimulatorTest, xor_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(8, "c", RIF_OUTPUT);
  builder.add_xor(c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0b11110000);
}

TEST(SimulatorTest, xnor_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(8, "c", RIF_OUTPUT);
  builder.add_xnor(c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0b00001111);
}

TEST(SimulatorTest, reg_expr) {
  // INPUT a
  // OUTPUT b
  // VAR a, b, c
  // IN
  // c = REG a
  // b = NOT c

  ProgramBuilder builder;
  auto a = builder.add_register(1, "a", RIF_INPUT);
  auto b = builder.add_register(1, "b", RIF_OUTPUT);
  auto c = builder.add_register(1, "c");
  builder.add_reg(c, a); // c = REG a
  builder.add_not(b, c); // b = NOT c
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 1);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(b), 1);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(b), 0);
}

TEST(SimulatorTest, concat_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(4, "a", RIF_INPUT);
  auto b = builder.add_register(3, "b", RIF_INPUT);
  auto c1 = builder.add_register(7, "c1", RIF_OUTPUT);
  auto c2 = builder.add_register(7, "c2", RIF_OUTPUT);
  builder.add_concat(c1, a, b);
  builder.add_concat(c2, b, a);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b1001);
  simulator.set_register(b, 0b010);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b1001);
  EXPECT_EQ(simulator.get_register(b), 0b010);
  EXPECT_EQ(simulator.get_register(c1), 0b0101001); // CONCAT a b
  EXPECT_EQ(simulator.get_register(c2), 0b1001010); // CONCAT b a
}

TEST(SimulatorTest, mux_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b = builder.add_register(8, "b", RIF_INPUT);
  auto c = builder.add_register(1, "c", RIF_INPUT);
  auto d = builder.add_register(8, "d", RIF_OUTPUT);
  builder.add_mux(d, c, a, b);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.set_register(b, 0b01101101);

  simulator.set_register(c, 1);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 1);
  EXPECT_EQ(simulator.get_register(d), 0b01101101); // b was selected

  simulator.set_register(c, 0);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b), 0b01101101);
  EXPECT_EQ(simulator.get_register(c), 0);
  EXPECT_EQ(simulator.get_register(d), 0b10011101); // a was selected
}

TEST(SimulatorTest, select_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b0 = builder.add_register(1, "b0", RIF_OUTPUT);
  auto b1 = builder.add_register(1, "b1", RIF_OUTPUT);
  auto b2 = builder.add_register(1, "b2", RIF_OUTPUT);
  builder.add_select(b0, 0, a);
  builder.add_select(b1, 1, a);
  builder.add_select(b2, 2, a);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b0), 1);
  EXPECT_EQ(simulator.get_register(b1), 0);
  EXPECT_EQ(simulator.get_register(b2), 1);
}

TEST(SimulatorTest, slice_expr) {
  ProgramBuilder builder;
  auto a = builder.add_register(8, "a", RIF_INPUT);
  auto b0 = builder.add_register(4, "b0", RIF_OUTPUT);
  auto b1 = builder.add_register(8, "b1", RIF_OUTPUT);
  auto b2 = builder.add_register(1, "b2", RIF_OUTPUT);
  builder.add_slice(b0, 0, 3, a);
  builder.add_slice(b1, 0, 7, a);
  builder.add_slice(b2, 2, 2, a);
  auto program = builder.build();
  ASSERT_NE(program, nullptr);

  // no scheduling needed.

  Simulator simulator(program);
  simulator.set_register(a, 0b10011101);
  simulator.cycle();
  EXPECT_EQ(simulator.get_register(a), 0b10011101);
  EXPECT_EQ(simulator.get_register(b0), 0b1101);
  EXPECT_EQ(simulator.get_register(b1), 0b10011101);
  EXPECT_EQ(simulator.get_register(b2), 1);
}
