#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "bytecode.hpp"

class MockByteCodeReader : public ByteCodeReader<MockByteCodeReader> {
public:
  using ByteCodeReader::ByteCodeReader;

  MOCK_METHOD(void, handle_nop, ());
  MOCK_METHOD(void, handle_break, ());
  MOCK_METHOD(void, handle_const, (const ConstInstruction &));
  MOCK_METHOD(void, handle_not, (const NotInstruction &));
  MOCK_METHOD(void, handle_and, (const BinaryInstruction &));
  MOCK_METHOD(void, handle_or, (const BinaryInstruction &));
  MOCK_METHOD(void, handle_nand, (const BinaryInstruction &));
  MOCK_METHOD(void, handle_nor, (const BinaryInstruction &));
  MOCK_METHOD(void, handle_xor, (const BinaryInstruction &));
  MOCK_METHOD(void, handle_reg, (const RegInstruction &));
  MOCK_METHOD(void, handle_slice, (const SliceInstruction &));
  MOCK_METHOD(void, handle_select, (const SelectInstruction &));
};

TEST(ByteCodeWriter, register_reg) {
  ByteCodeWriter writer;
  writer.register_reg();
  writer.register_reg(15);
  writer.register_reg(8);
  ByteCode bytecode = writer.finish();

  ASSERT_EQ(bytecode.registers.size(), 3);
  EXPECT_EQ(bytecode.registers[0].bit_width, 1);
  EXPECT_EQ(bytecode.registers[1].bit_width, 15);
  EXPECT_EQ(bytecode.registers[2].bit_width, 8);
}

TEST(ByteCodeWriter, write_nop) {
  ByteCodeWriter writer;
  writer.write_nop();
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_nop());
  reader.read_all();
}

// NOTE: write_break() does not exist, and therefore is not tested here, because
// no one should write BREAK instruction except the debugger.

TEST(ByteCodeWriter, write_const) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  writer.write_const(ConstInstruction{r0, UINT64_MAX});
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_const(ConstInstruction{r0, UINT64_MAX}));
  reader.read_all();
}

TEST(ByteCodeWriter, write_not) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(4);
  writer.write_not(NotInstruction{r0, r1});
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_not(NotInstruction{r0, r1}));
  reader.read_all();
}

TEST(ByteCodeWriter, write_binary_inst) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(2);
  const auto r1 = writer.register_reg(2);
  const auto r2 = writer.register_reg(2);
  writer.write_and(BinaryInstruction{r1, r2, r0});
  writer.write_or(BinaryInstruction{r2, r0, r1});
  writer.write_nand(BinaryInstruction{r0, r1, r2});
  writer.write_nor(BinaryInstruction{r1, r2, r0});
  writer.write_xor(BinaryInstruction{r2, r0, r1});
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_and(BinaryInstruction{r1, r2, r0}));
  EXPECT_CALL(reader, handle_or(BinaryInstruction{r2, r0, r1}));
  EXPECT_CALL(reader, handle_nand(BinaryInstruction{r0, r1, r2}));
  EXPECT_CALL(reader, handle_nor(BinaryInstruction{r1, r2, r0}));
  EXPECT_CALL(reader, handle_xor(BinaryInstruction{r2, r0, r1}));
  reader.read_all();
}

TEST(ByteCodeWriter, write_reg) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(2);
  const auto r1 = writer.register_reg(2);
  writer.write_reg(RegInstruction{r1, r0});
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_reg(RegInstruction{r1, r0}));
  reader.read_all();
}

TEST(ByteCodeWriter, write_slice) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(4);
  const auto r1 = writer.register_reg(10);
  writer.write_slice(SliceInstruction{r0, 4, 7, r1});
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_slice(SliceInstruction{r0, 4, 7, r1}));
  reader.read_all();
}

TEST(ByteCodeWriter, write_select) {
  ByteCodeWriter writer;
  const auto r0 = writer.register_reg(1);
  const auto r1 = writer.register_reg(20);
  writer.write_select(SelectInstruction{r0, 5, r1});
  ByteCode bytecode = writer.finish();

  MockByteCodeReader reader(bytecode);
  EXPECT_CALL(reader, handle_select(SelectInstruction{r0, 5, r1}));
  reader.read_all();
}
