#include <gtest/gtest.h>

#include "disassembler.hpp"

TEST(DisassemblerTest, registers) {
  ProgramBuilder builder;
  (void)builder.add_register(5, "foo", RIF_INPUT);
  (void)builder.add_register(2, {}, RIF_INPUT);
  (void)builder.add_register(1, "bar", RIF_OUTPUT);
  (void)builder.add_register(28, {}, RIF_OUTPUT);
  (void)builder.add_register(15, "b");
  (void)builder.add_register();
  auto program = builder.build();

  std::stringstream out;
  Disassembler::disassemble(program, out);
  EXPECT_EQ(out.str(), R"(INPUT foo, __r1
OUTPUT bar, __r3
VAR foo:5, __r1:2, bar:1, __r3:28, b:15, __r5:1
IN
)");
}

TEST(DisassemblerTest, constants) {
  ProgramBuilder builder;
  (void)builder.add_register(1, {}, RIF_INPUT);
  const auto o1 = builder.add_register(1, "o1", RIF_OUTPUT);
  const auto o2 = builder.add_register(4, "o2", RIF_OUTPUT);
  const auto o3 = builder.add_register(8, "o3", RIF_OUTPUT);
  builder.add_const(o1, 0);
  builder.add_const(o2, 1);
  builder.add_const(o3, 0b10110010);
  auto program = builder.build();

  std::stringstream out;
  Disassembler::disassemble(program, out);
  EXPECT_EQ(out.str(), R"(INPUT __r0
OUTPUT o1, o2, o3
VAR __r0:1, o1:1, o2:4, o3:8
IN
o1 = 0
o2 = 0001
o3 = 10110010
)");
}

TEST(DisassemblerTest, load_expression) {
  ProgramBuilder builder;
  const auto a = builder.add_register(1, "a", RIF_INPUT);
  const auto o1 = builder.add_register(1, "o1", RIF_OUTPUT);
  builder.add_load(o1, a);
  auto program = builder.build();

  std::stringstream out;
  Disassembler::disassemble(program, out);
  EXPECT_EQ(out.str(), R"(INPUT a
OUTPUT o1
VAR a:1, o1:1
IN
o1 = a
)");
}

TEST(DisassemblerTest, other_expressions) {
  ProgramBuilder builder;
  const auto a = builder.add_register(1, "a", RIF_INPUT);
  const auto b = builder.add_register(4, "b", RIF_INPUT);
  const auto c = builder.add_register(4, "c", RIF_INPUT);
  const auto o1 = builder.add_register(1, "o1", RIF_OUTPUT);
  const auto o2 = builder.add_register(1, "o2", RIF_OUTPUT);
  const auto o3 = builder.add_register(1, "o3", RIF_OUTPUT);
  const auto o4 = builder.add_register(3, "o4", RIF_OUTPUT);
  const auto o5 = builder.add_register(5, "o5", RIF_OUTPUT);
  const auto o6 = builder.add_register(1, "o6", RIF_OUTPUT);
  const auto o7 = builder.add_register(4, "o7", RIF_OUTPUT);
  builder.add_reg(o1, a);
  builder.add_select(o2, 0, a);
  builder.add_select(o3, 3, a);
  builder.add_slice(o4, 1, 3, b);
  builder.add_concat(o5, a, b);
  builder.add_not(o6, a);
  builder.add_mux(o7, a, b, c);
  auto program = builder.build();

  std::stringstream out;
  Disassembler::disassemble(program, out);
  EXPECT_EQ(out.str(), R"(INPUT a, b, c
OUTPUT o1, o2, o3, o4, o5, o6, o7
VAR a:1, b:4, c:4, o1:1, o2:1, o3:1, o4:3, o5:5, o6:1, o7:4
IN
o1 = REG a
o2 = SELECT 0 a
o3 = SELECT 3 a
o4 = SLICE 1 3 b
o5 = CONCAT a b
o6 = NOT a
o7 = MUX a b c
)");
}

TEST(DisassemblerTest, binary_expressions) {
  ProgramBuilder builder;
  const auto a = builder.add_register(1, "a", RIF_INPUT);
  const auto b = builder.add_register(1, "b", RIF_INPUT);
  const auto o1 = builder.add_register(1, "o1", RIF_OUTPUT);
  const auto o2 = builder.add_register(1, "o2", RIF_OUTPUT);
  const auto o3 = builder.add_register(1, "o3", RIF_OUTPUT);
  const auto o4 = builder.add_register(1, "o4", RIF_OUTPUT);
  const auto o5 = builder.add_register(1, "o5", RIF_OUTPUT);
  const auto o6 = builder.add_register(1, "o6", RIF_OUTPUT);
  builder.add_and(o1, a, b);
  builder.add_nand(o2, a, b);
  builder.add_or(o3, a, b);
  builder.add_nor(o4, a, b);
  builder.add_xor(o5, a, b);
  builder.add_xnor(o6, a, b);
  auto program = builder.build();

  std::stringstream out;
  Disassembler::disassemble(program, out);
  EXPECT_EQ(out.str(), R"(INPUT a, b
OUTPUT o1, o2, o3, o4, o5, o6
VAR a:1, b:1, o1:1, o2:1, o3:1, o4:1, o5:1, o6:1
IN
o1 = AND a b
o2 = NAND a b
o3 = OR a b
o4 = NOR a b
o5 = XOR a b
o6 = XNOR a b
)");
}

TEST(DisassemblerTest, ram_rom) {
  ProgramBuilder builder;
  const auto read_addr = builder.add_register(8, "read_addr", RIF_INPUT);
  const auto write_enable = builder.add_register(1, "write_enable", RIF_INPUT);
  const auto write_addr = builder.add_register(8, "write_addr", RIF_INPUT);
  const auto write_data = builder.add_register(16, "write_data", RIF_INPUT);
  const auto o1 = builder.add_register(16, "o1", RIF_OUTPUT);
  const auto o2 = builder.add_register(16, "o2", RIF_OUTPUT);
  builder.add_rom(o1, 8, 16, read_addr);
  builder.add_ram(o2, 8, 16, read_addr, write_enable, write_addr, write_data);
  auto program = builder.build();

  std::stringstream out;
  Disassembler::disassemble(program, out);
  EXPECT_EQ(out.str(), R"(INPUT read_addr, write_enable, write_addr, write_data
OUTPUT o1, o2
VAR read_addr:8, write_enable:1, write_addr:8, write_data:16, o1:16, o2:16
IN
o1 = ROM 8 16 read_addr
o2 = RAM 8 16 read_addr write_enable write_addr write_data
)");
}
