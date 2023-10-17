#ifndef NETLIST_PROGRAM_HPP
#define NETLIST_PROGRAM_HPP

#include <string_view>
#include <stdexcept>
#include <vector>
#include <memory>
#include <unordered_map>

#include "parser.hpp"

#define delete_copy_ctr(class) class(class &) = delete;\
class(const class &) = delete;

#define same_bus_size(bus1, bus2) ((bus1)->get_bus_size() == (bus2)->get_bus_size()) ? (bus1)->get_bus_size() : \
throw std::invalid_argument("Bus size mismatch")

#define check_bus_size_equal(bus, size) ((bus)->get_bus_size() == (size)) ? std::move(bus) : \
throw std::invalid_argument("Wrong bus size")

#define check_bus_size_at_least(bus, size) ((bus)->get_bus_size() >= (size)) ? std::move(bus) : \
throw std::invalid_argument("Bus size too small")

typedef int bus_size_t;
typedef std::uint_least64_t value_t;
typedef std::string_view ident_t;

class Bus {
public:
  /// Returns the size of the underlying bus.
  [[nodiscard]] bus_size_t get_bus_size() const noexcept { return bus_size; }

  delete_copy_ctr(Bus)
protected:
  explicit Bus(const bus_size_t size) : bus_size(size) {
    if (size <= 0) {
      throw std::invalid_argument("Cannot have a negative or null bus size.");
    }

    if (size > sizeof(value_t) * 8) {
      throw std::invalid_argument("Bus size too large to be represented.");
    }
  }

private:
  const bus_size_t bus_size;
};

/// The base class for values.
///
/// There is two kinds of value: constants and variables.
class Argument : public Bus {
public:
  enum class Kind {
    CONSTANT,
    VARIABLE,
  };

  /// Kind of the Argument : Constants or Variables
  [[nodiscard]] virtual Kind get_kind() const noexcept = 0;

  // Delete copy constructor and added a virtual destructor.
  delete_copy_ctr(Argument)
  virtual ~Argument() noexcept = default;

protected:
  explicit Argument(const bus_size_t size) :
      Bus(size) {}
};

/// Represents a constant in the source code such as 't' or '42'.
class Constant : public Argument {
  static constexpr Kind KIND = Kind::CONSTANT;
public:
  /// Constructor of a 'bus constant'
  /// \param size Size of the bus
  /// \param value Value of the constant. Must be in [0, 2^size - 1]
  explicit Constant(const bus_size_t size, value_t value) :
      Argument(size), m_value(value < 1 << size ? value :
                              throw std::invalid_argument("Constant bus value too large.")) {}

  /// Constructor for a single bit constant : True or False.
  /// \param value Boolean value of the bit
  explicit Constant(bool value) :
      Argument(1), m_value(value ? 1 : 0) {}

  /// Kind of a Constant (obvious)
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value of the constant.
  [[nodiscard]] value_t get_value() const noexcept { return m_value; }

  delete_copy_ctr(Constant)
private:
  const value_t m_value;
};

/// Represent a variable such a 'a' or '_l_245'
class Variable : public Argument {
  static constexpr Kind KIND = Kind::VARIABLE;
public:
  /// Constructor of a variable
  /// \param size Bus size of the variable
  /// \param name Name identifier of the variable
  explicit Variable(const bus_size_t size, ident_t name) :
      Argument(size), m_name(name) {}

  /// Kind of a Variable (obvious)
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the name of the variable
  [[nodiscard]] ident_t get_name() const noexcept { return m_name; }

  delete_copy_ctr(Variable)
private:
  const ident_t m_name;
};

/// The base class to all expressions.
class Expression : public Bus {
public:
  enum class Kind { ARG, REG, NOT, BINOP, MUX, ROM, RAM, CONCAT, SLICE, SELECT };

  /// Get the kind of Expression
  [[nodiscard]] virtual Kind get_kind() const noexcept = 0;

  // Delete copy constructor and added a virtual destructor.
  delete_copy_ctr(Expression)
  virtual ~Expression() noexcept = default;

protected:
  explicit Expression(const bus_size_t size) :
      Bus(size) {};

  using arg_ptr = std::unique_ptr<Argument>;
  using var_ptr = std::unique_ptr<Variable>;
};

/// The 'arg' expression. (Direct association such as _l_10 = z or ba = 42)
class ArgExpression : public Expression {
  static constexpr Kind KIND = Kind::ARG;
public:
  /// Constructor of a Arg Expression. This expression can be used to manually set
  /// a value to a variable. Example : _l_10 = 5.
  /// Beware a variable can be passed as an argument. It correspond to the case : _e = _l_5.
  /// \param arg Argument of the expression
  explicit ArgExpression(arg_ptr arg) :
      Expression(arg->get_bus_size()), m_value(std::move(arg)) {}

  /// ArgExpression kind : Kind::ARG
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the argument of this expression.
  [[nodiscard]] const Argument &get_argument() const noexcept { return *m_value; }

  delete_copy_ctr(ArgExpression)
private:
  const arg_ptr m_value;
};

/// The 'REG variable' expression.
class RegExpression : public Expression {
  static constexpr Kind KIND = Kind::REG;
public:
  /// Constructor of a Register operation. This operation can be used
  /// to get the last cycle value of a variable.
  /// \param var Variable identifier used
  explicit RegExpression(var_ptr var) :
      Expression(var->get_bus_size()), m_var(std::move(var)) {}

  /// RegExpression kind : Kind::REG
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value to to which the operator applies.
  [[nodiscard]] const Variable &get_variable() const noexcept { return *m_var; }

  delete_copy_ctr(RegExpression)
private:
  const var_ptr m_var;
};

/// The 'NOT arg' expression.
class NotExpression : public Expression {
  static constexpr Kind KIND = Kind::NOT;
public:
  explicit NotExpression(arg_ptr arg) :
      Expression(arg->get_bus_size()), m_arg(std::move(arg)) {}

  /// NotExpression Kind : Kind::NOT
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value to to which the operator applies.
  [[nodiscard]] const Argument &get_argument() const { return *m_arg; }

  delete_copy_ctr(NotExpression)
private:
  const arg_ptr m_arg;
};

/// The 'binop lhs rhs' expression.
class BinOpExpression : public Expression {
  static constexpr Kind KIND = Kind::BINOP;
public:
  enum class BinOpKind { OR, XOR, AND, NAND };
  /// Constructor of a Binary Expression
  /// \param binop Type of operation performed : AND, OR, etc.
  /// \param lhs Left hand side of the operation
  /// \param rhs Right hand side of the operation
  explicit BinOpExpression(BinOpKind binop, arg_ptr lhs, arg_ptr rhs) :
      Expression(same_bus_size(lhs, rhs)),
      m_kind(binop),
      m_lhs(std::move(lhs)),
      m_rhs(std::move(rhs)) {}

  /// BinOpExpression Kind : Kind::BINOP
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Get the Binary Operation Kind : OR, AND, etc.
  [[nodiscard]] BinOpKind get_binop_kind() const noexcept { return m_kind; }

  /// Returns the left value to to which the operator applies.
  [[nodiscard]] const Argument &get_lhs_argument() const noexcept { return *m_lhs; }

  /// Returns the right value to to which the operator applies.
  [[nodiscard]] const Argument &get_rhs_argument() const noexcept { return *m_rhs; }

  delete_copy_ctr(BinOpExpression)
private:
  const BinOpKind m_kind;
  const arg_ptr m_lhs;
  const arg_ptr m_rhs;
};

/// The 'MUX choice true_arg false_arg' expression.
class MuxExpression : public Expression {
  static constexpr Kind KIND = Kind::MUX;
public:
  /// Constructor of a MuxExpression
  /// \param choice Bit used to make a choice. Must have a bus size of 1
  /// \param true_arg Value of the expression if choice bit is 1
  /// \param false_arg Value of the expression if choice bit is 0
  explicit MuxExpression(arg_ptr choice, arg_ptr true_arg, arg_ptr false_arg) :
      Expression(same_bus_size(true_arg, false_arg)),
      m_choice(check_bus_size_equal(choice, 1)),
      m_true(std::move(true_arg)),
      m_false(std::move(false_arg)) {}

  /// MuxExpression Kind : Kind::MUX
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value used to make a choice.
  [[nodiscard]] const Argument &get_choice_argument() const noexcept { return *m_choice; }

  /// Returns the value used if the choice bit is true.
  [[nodiscard]] const Argument &get_true_argument() const noexcept { return *m_true; }

  /// Returns the value used if the choice bit is false.
  [[nodiscard]] const Argument &get_false_argument() const noexcept { return *m_false; }

  delete_copy_ctr(MuxExpression)
private:
  const arg_ptr m_choice;
  const arg_ptr m_true;
  const arg_ptr m_false;
};

/// The 'ROM addr_size word_size read_addr' expression.
class RomExpression : public Expression {
  static constexpr Kind KIND = Kind::ROM;
public:
  /// Constructor of a RomExpression
  /// \param addr_size The size of an address bus
  /// \param word_size The size of the word read
  /// \param read_addr The address to read to
  explicit RomExpression(const bus_size_t addr_size, const bus_size_t word_size, arg_ptr read_addr) :
      Expression(word_size),
      m_read_addr(check_bus_size_equal(read_addr, addr_size)) {}

  /// RomExpression Kind : Kind::ROM
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns read address
  [[nodiscard]] const Argument &get_read_address() const noexcept { return *m_read_addr; }

  /// Returns the bus size of the read address
  [[nodiscard]] bus_size_t get_address_size() const noexcept { return m_read_addr->get_bus_size(); }

  delete_copy_ctr(RomExpression)
private:
  const arg_ptr m_read_addr;
};

/// The 'RAM addr_size word_size read_addr write_enable write_addr data' expression.
class RamExpression : public Expression {
  static constexpr Kind KIND = Kind::RAM;
public:
  /// Constructor of a RamExpression
  /// \param addr_size The size of an address bus
  /// \param word_size The size of the word read
  /// \param read_addr The address to read to. Must have a bus size of addr_size.
  /// \param write_enable Bit used to know if writing. If 1, we write 'data' at 'write_addr'.
  /// \param write_addr The address to write to. Must have a bus size of addr_size.
  /// \param data Data to write (if write_enable). Must have a bus size of word_size.
  explicit RamExpression(const bus_size_t addr_size, const bus_size_t word_size, arg_ptr read_addr,
                         arg_ptr write_enable, arg_ptr write_addr, arg_ptr data) :
      Expression(word_size),
      m_read_addr(check_bus_size_equal(read_addr, addr_size)),
      m_write_enable(check_bus_size_equal(write_enable, 1)),
      m_write_addr(check_bus_size_equal(write_addr, addr_size)),
      m_write_data(check_bus_size_equal(data, word_size)) {}

  /// RamExpression Kind : Kind::RAM
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the bus size of the read and write address
  [[nodiscard]] bus_size_t get_address_size() const noexcept { return m_read_addr->get_bus_size(); }

  /// Returns read address
  [[nodiscard]] const Argument &get_read_address() const noexcept { return *m_read_addr; }

  /// Returns write enable bit flag
  [[nodiscard]] const Argument &get_write_enable() const noexcept { return *m_write_enable; }

  /// Returns write address
  [[nodiscard]] const Argument &get_write_address() const noexcept { return *m_write_addr; }

  /// Returns write data argument
  [[nodiscard]] const Argument &get_write_data() const noexcept { return *m_write_data; }

  delete_copy_ctr(RamExpression)
private:
  const arg_ptr m_read_addr;
  const arg_ptr m_write_enable;
  const arg_ptr m_write_addr;
  const arg_ptr m_write_data;
};

/// The 'CONCAT begin_arg end_arg' expression.
class ConcatExpression : public Expression {
  static constexpr Kind KIND = Kind::CONCAT;
public:
  /// Constructor of the concatenation of two buses.
  /// \param begin_arg First part of the concatenation. It will be in the beginning of the created bus.
  /// \param end_arg Lst part of the concatenation. It will be at the end of the created bus.
  explicit ConcatExpression(arg_ptr begin_arg, arg_ptr end_arg) :
      Expression(begin_arg->get_bus_size() + end_arg->get_bus_size()),
      m_beg(std::move(begin_arg)),
      m_last(std::move(end_arg)) {}

  /// ConcatExpression Kind : Kind::CONCAT
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the beginning of the bus.
  [[nodiscard]] const Argument &get_beginning_part() const noexcept { return *m_beg; }

  /// Returns the end of the bus.
  [[nodiscard]] const Argument &get_last_part() const noexcept { return *m_last; }

  delete_copy_ctr(ConcatExpression)
private:
  const arg_ptr m_beg;
  const arg_ptr m_last;
};

/// The 'SLICE begin end arg' expression.
class SliceExpression : public Expression {
  static constexpr Kind KIND = Kind::SLICE;
public:
  /// Constructor for the operation Slice. It value is bus beginning at the 'begin'-th bit of 'arg'
  /// to the 'end'-th bit og 'arg' INCLUDED.
  /// \param begin Begin index of the slice section.
  /// \param end End index of the slice section INCLUDED.
  /// \param arg Argument to slice
  explicit SliceExpression(const bus_size_t begin, const bus_size_t end, arg_ptr arg) :
      Expression(end - begin + 1),
      m_begin(begin),
      m_end(end),
      m_arg(check_bus_size_at_least(arg, end + 1)) {} // indexes start at 0 !

  /// SliceExpression Kind : Kind::SLICE
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the beginning index of the slice.
  [[nodiscard]]  bus_size_t get_begin_index() const noexcept { return m_begin; }

  /// Returns the ending index of the slice INCLUDED.
  [[nodiscard]] bus_size_t get_end_index() const noexcept { return m_end; }

  /// Returns the argument.
  [[nodiscard]] const Argument &get_argument() const noexcept { return *m_arg; }

  delete_copy_ctr(SliceExpression)
private:
  const bus_size_t m_begin;
  const bus_size_t m_end;
  const arg_ptr m_arg;
};

/// The 'SELECT index arg' expression.
class SelectExpression : public Expression {
  static constexpr Kind KIND = Kind::SELECT;
public:
  /// Constructor of the Select expression. Select the 'index'-th bit of the bus 'arg'.
  /// \param index Bit index to select
  /// \param arg Argument
  explicit SelectExpression(const bus_size_t index, arg_ptr arg) :
      Expression(1),
      m_index(index),
      m_arg(check_bus_size_at_least(arg, index + 1)) {} // indexes start at 0 !

  /// SelectExpression Kind : Kind::SELECT
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the index of the bit selected.
  [[nodiscard]]  bus_size_t get_begin_index() const noexcept { return m_index; }

  /// Returns the argument.
  [[nodiscard]] const Argument &get_argument() const noexcept { return *m_arg; }

  delete_copy_ctr(SelectExpression)
private:
  const bus_size_t m_index;
  const arg_ptr m_arg;
};

using Equation = std::pair<Variable, std::unique_ptr<Expression>>;

struct Program {
  const std::vector<Variable> input;
  const std::vector<std::reference_wrapper<Variable>> output;
  const std::vector<Equation> equations;
};

//template<typename Derived> class Visitor {
//public:
//#define DISPATCH(call) (static_cast<Derived *>(this)->call)
//
//  void visit(Value *value) {
//    assert(value != nullptr);
//
//    switch (value->get_kind()) {
//    case Value::Kind::CONSTANT:return DISPATCH(visit_constant(static_cast<Constant *>(value)));
//    case Value::Kind::INPUT:return DISPATCH(visit_input(static_cast<Input *>(value)));
//    case Value::Kind::EQUATION:return DISPATCH(visit_equation(static_cast<Equation *>(value)));
//    }
//  }
//
//  void visit_value(Value *value) {}
//  void visit_constant(Constant *value) { return DISPATCH(visit_value(value)); }
//  void visit_input(Input *value) { return DISPATCH(visit_value(value)); }
//  void visit_equation(Equation *value) { return DISPATCH(visit_value(value)); }
//
//  void visit(Expression *expr) {
//    assert(expr != nullptr);
//
//    switch (expr->get_kind()) {
//    case Expression::Kind::NOT:return DISPATCH(visit_not_expr(static_cast<NotExpression *>(expr)));
//    case Expression::Kind::BINARY:return DISPATCH(visit_binary_expr(static_cast<BinaryExpression *>(expr)));
//    }
//  }
//
//  void visit_expr(Expression *expr) {}
//  void visit_not_expr(NotExpression *expr) {
//    return DISPATCH(visit_expr(expr));
//  }
//  void visit_binary_expr(BinaryExpression *expr) {
//    return DISPATCH(visit_expr(expr));
//  }
//};

#endif // NETLIST_PROGRAM_HPP
