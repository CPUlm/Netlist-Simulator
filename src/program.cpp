#include "program.hpp"

Bus::Bus(const bus_size_t size) : bus_size(size) {
  if (size <= 0) {
    throw std::invalid_argument("Cannot have a negative or null bus size.");
  }

  if (size > max_bus_size) {
    throw std::invalid_argument("Bus size too large to be represented.");
  }
}

Constant::Constant(const bus_size_t size, value_t value)
    : Argument(size),
      m_value(value) {
  if (value > Bus::max_value(size)) {
    throw std::invalid_argument("Constant bus too small for the given value.");
  }
}

Variable::Variable(const bus_size_t size, ident_t name) : Argument(size), m_name(name) {}

static inline void assert_same_bus_size(const Argument::ptr &lhs, const Argument::ptr &rhs) {
  if (lhs->get_bus_size() != rhs->get_bus_size()) {
    throw std::invalid_argument("Bus size mismatch");
  }
}

static inline void assert_bus_size_eq(const Argument::ptr &arg, bus_size_t size) {
  if (arg->get_bus_size() != size) {
    throw std::invalid_argument("Wrong bus size");
  }
}

static inline void assert_bus_size_gt(const Argument::ptr &arg, bus_size_t size) {
  if (arg->get_bus_size() <= size) {
    throw std::invalid_argument("Bus size too small");
  }
}

BinOpExpression::BinOpExpression(BinOpExpression::BinOpKind binop, const Argument::ptr &lhs, const Argument::ptr &rhs) :
    Expression(lhs->get_bus_size()),
    m_kind(binop),
    m_lhs(lhs),
    m_rhs(rhs) {
  assert_same_bus_size(lhs, rhs);
}

MuxExpression::MuxExpression(const Argument::ptr &choice, const Argument::ptr &true_arg, const Argument::ptr &false_arg)
    : Expression(true_arg->get_bus_size()),
      m_choice(choice),
      m_true(true_arg),
      m_false(false_arg) {
  assert_same_bus_size(true_arg, false_arg);
  assert_bus_size_eq(choice, 1);
}

RomExpression::RomExpression(const bus_size_t addr_size, const bus_size_t word_size, const Argument::ptr &read_addr) :
    Expression(word_size),
    m_read_addr(read_addr) {
  assert_bus_size_eq(read_addr, addr_size);
}

RamExpression::RamExpression(const bus_size_t addr_size,
                             const bus_size_t word_size,
                             const Argument::ptr &read_addr,
                             const Argument::ptr &write_enable,
                             const Argument::ptr &write_addr,
                             const Argument::ptr &data) :
    Expression(word_size),
    m_read_addr(read_addr),
    m_write_enable(write_enable),
    m_write_addr(write_addr),
    m_write_data(data) {
  assert_bus_size_eq(read_addr, addr_size);
  assert_bus_size_eq(write_enable, 1);
  assert_bus_size_eq(write_addr, addr_size);
  assert_bus_size_eq(data, word_size);
}

ConcatExpression::ConcatExpression(const Argument::ptr &begin_arg, const Argument::ptr &end_arg) :
    Expression(begin_arg->get_bus_size() + end_arg->get_bus_size()),
    m_beg(begin_arg),
    m_last(end_arg) {}

SliceExpression::SliceExpression(const bus_size_t begin, const bus_size_t end, const Argument::ptr &arg) :
    Expression(end - begin + 1),
    m_begin(begin),
    m_end(end),
    m_arg(arg) {
  assert_bus_size_gt(arg, end);
}

SelectExpression::SelectExpression(const bus_size_t index, const Argument::ptr &arg) :
    Expression(1),
    m_index(index),
    m_arg(arg) {
  assert_bus_size_gt(arg, index);
}


