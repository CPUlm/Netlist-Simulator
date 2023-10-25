#include <gtest/gtest.h>

#include "simulator.hpp"

TEST(SimulatorTest, step) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  writer.write_const({r0, 0b0001});
  writer.write_const({r0, 0b0010});
  writer.write_const({r0, 0b0100});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b0000);
  ASSERT_EQ(simulator.get_register_count(), 1);

  EXPECT_EQ(simulator.get_register(r0), 0b0000);
  simulator.step();
  EXPECT_EQ(simulator.get_register(r0), 0b0001);
  simulator.step();
  EXPECT_EQ(simulator.get_register(r0), 0b0010);
  simulator.step();
  EXPECT_EQ(simulator.get_register(r0), 0b0100);
  EXPECT_TRUE(simulator.at_end());
}

TEST(SimulatorTest, instruction_nop) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  writer.write_nop();
  writer.write_nop();
  writer.write_nop();
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b1010);
  simulator.set_register(r1, 0b0110);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 2);
  // r0 and r1 are registers unused in the bytecode -> unmodified
  EXPECT_EQ(simulator.get_register(r0), 0b1010);
  EXPECT_EQ(simulator.get_register(r1), 0b0110);
}

TEST(SimulatorTest, instruction_const) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  writer.write_const({r0, 0b1111});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b1010);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 1);
  // r0 is the output register -> must be modified
  EXPECT_EQ(simulator.get_register(r0), 0b1111);
}

TEST(SimulatorTest, instruction_not) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  writer.write_not({r1, r0});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b1010);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 2);
  // r0 is the input register -> must be unmodified
  // r1 is the output register -> must contain ~r0
  EXPECT_EQ(simulator.get_register(r0), 0b1010);
  EXPECT_EQ(simulator.get_register(r1), 0b0101);
}

TEST(SimulatorTest, instruction_and) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  const auto r2 = writer.register_reg(4);
  writer.write_and({r2, r0, r1});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b0011);
  simulator.set_register(r1, 0b0101);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 3);
  // r0, r1 are the input registers -> must be unmodified
  // r2 is the output register -> must contain r0 & r1
  EXPECT_EQ(simulator.get_register(r0), 0b0011);
  EXPECT_EQ(simulator.get_register(r1), 0b0101);
  EXPECT_EQ(simulator.get_register(r2), 0b0001);
}

TEST(SimulatorTest, instruction_or) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  const auto r2 = writer.register_reg(4);
  writer.write_or({r2, r0, r1});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b0011);
  simulator.set_register(r1, 0b0101);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 3);
  // r0, r1 are the input registers -> must be unmodified
  // r2 is the output register -> must contain r0 | r1
  EXPECT_EQ(simulator.get_register(r0), 0b0011);
  EXPECT_EQ(simulator.get_register(r1), 0b0101);
  EXPECT_EQ(simulator.get_register(r2), 0b0111);
}

TEST(SimulatorTest, instruction_nand) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  const auto r2 = writer.register_reg(4);
  writer.write_nand({r2, r0, r1});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b0011);
  simulator.set_register(r1, 0b0101);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 3);
  // r0, r1 are the input registers -> must be unmodified
  // r2 is the output register -> must contain r0 NAND r1
  EXPECT_EQ(simulator.get_register(r0), 0b0011);
  EXPECT_EQ(simulator.get_register(r1), 0b0101);
  EXPECT_EQ(simulator.get_register(r2), 0b1110);
}

TEST(SimulatorTest, instruction_nor) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  const auto r2 = writer.register_reg(4);
  writer.write_nor({r2, r0, r1});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b0011);
  simulator.set_register(r1, 0b0101);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 3);
  // r0, r1 are the input registers -> must be unmodified
  // r2 is the output register -> must contain r0 NOR r1
  EXPECT_EQ(simulator.get_register(r0), 0b0011);
  EXPECT_EQ(simulator.get_register(r1), 0b0101);
  EXPECT_EQ(simulator.get_register(r2), 0b1000);
}

TEST(SimulatorTest, instruction_slice) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(8);
  const auto r1 = writer.register_reg(4);
  const auto r2 = writer.register_reg(8);
  const auto r3 = writer.register_reg(2);
  writer.write_slice({r1, 2, 5, r0});
  writer.write_slice({r2, 0, 7, r0});
  writer.write_slice({r3, 2, 3, r1});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b01110010);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 4);
  // r0 is the input register -> must be unmodified
  // r1,r2,r3 are the output registers
  EXPECT_EQ(simulator.get_register(r0), 0b01110010);
  EXPECT_EQ(simulator.get_register(r1), 0b1100);
  EXPECT_EQ(simulator.get_register(r2), 0b01110010);
  EXPECT_EQ(simulator.get_register(r3), 0b11);
}

TEST(SimulatorTest, instruction_select) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(2);
  const auto r1 = writer.register_reg(1);
  const auto r2 = writer.register_reg(1);
  writer.write_select({r1, 0, r0});
  writer.write_select({r2, 1, r0});
  ByteCode bytecode = writer.finish();

  Simulator simulator(bytecode);
  simulator.set_register(r0, 0b10);
  simulator.execute();

  ASSERT_EQ(simulator.get_register_count(), 3);
  // r0 is the input register -> must be unmodified
  // r1,r2 are the output registers
  EXPECT_EQ(simulator.get_register(r0), 0b10);
  EXPECT_EQ(simulator.get_register(r1), 0);
  EXPECT_EQ(simulator.get_register(r2), 1);
}
