#ifndef NETLIST_PROGRAM_HPP
#define NETLIST_PROGRAM_HPP

#include <string_view>
#include <stdexcept>
#include <vector>
#include <memory>
#include <cassert>
#include <unordered_map>
#include <limits>

#define delete_copy_ctr(class) class(class &) = delete;\
class(const class &) = delete;

typedef int bus_size_t;
typedef std::uint_least64_t value_t;
typedef std::string_view ident_t;

constexpr bus_size_t max_bus_size = sizeof(value_t) * 8;

class Bus {
public:
  /// Returns the size of the underlying bus.
  [[nodiscard]] bus_size_t get_bus_size() const noexcept { return bus_size; }

  /// Returns the max value of a bus with size s
  [[nodiscard]] static inline value_t max_value(bus_size_t s) noexcept {
    if (s == max_bus_size) {
      return std::numeric_limits<value_t>::max();
    } else {
      return (static_cast<value_t>(1) << s) - static_cast<value_t>(1);
    }
  };

  delete_copy_ctr(Bus)

protected:
  explicit Bus(bus_size_t size);

private:
  const bus_size_t bus_size;
};

/// The base class for values.
///
/// There is two kinds of value: constants and variables.
class Argument : public Bus {
public:
  using ptr = std::shared_ptr<const Argument>;

  enum class Kind {
    CONSTANT,
    VARIABLE,
  };

  /// Kind of the Argument : Constants or Variables
  [[nodiscard]] virtual Kind get_kind() const noexcept = 0;

  /// A representation of the Argument (value for Constant, name for Variables)
  [[nodiscard]] virtual std::string get_repr() const noexcept = 0;

  // Delete copy constructor and added a virtual destructor.
  delete_copy_ctr(Argument)

  virtual ~Argument() noexcept = default;

protected:
  explicit Argument(const bus_size_t size) :
      Bus(size) {}
};

/// Represents a constant in the source code such as '0d42' or '10010' or '0x5fe'.
class Constant : public Argument {
  static constexpr Kind KIND = Kind::CONSTANT;
public:
  using ptr = std::shared_ptr<const Constant>;

  /// Constructor of a 'bus constant'
  /// \param size Size of the bus
  /// \param value Value of the constant. Must be in [0, 2^size - 1]
  explicit Constant(bus_size_t size, value_t value);

  /// Kind of a Constant (obvious)
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value of the constant.
  [[nodiscard]] value_t get_value() const noexcept { return m_value; }

  [[nodiscard]] std::string get_repr() const noexcept override {
    return std::to_string(get_value());
  }

  delete_copy_ctr(Constant)

private:
  const value_t m_value;
};

/// Represent a variable such a 'a' or '_l_245'
class Variable : public Argument {
  static constexpr Kind KIND = Kind::VARIABLE;
public:
  using ptr = std::shared_ptr<Variable>;

  /// Constructor of a variable
  /// \param size Bus size of the variable
  /// \param name Name identifier of the variable
  explicit Variable(bus_size_t size, ident_t name);

  /// Kind of a Variable (obvious)
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the name of the variable
  [[nodiscard]] ident_t get_name() const noexcept { return m_name; }

  [[nodiscard]] std::string get_repr() const noexcept override {
    return std::string(get_name());
  }

  delete_copy_ctr(Variable)

private:
  const ident_t m_name;
};

/// The base class to all expressions.
class Expression : public Bus {
public:
  typedef std::unique_ptr<Expression> ptr;

  enum class Kind {
    ARG, REG, NOT, BINOP, MUX, ROM, RAM, CONCAT, SLICE, SELECT
  };

  /// Get the kind of Expression
  [[nodiscard]] virtual Kind get_kind() const noexcept = 0;

  // Delete copy constructor and added a virtual destructor.
  delete_copy_ctr(Expression)

  virtual ~Expression() noexcept = default;

protected:
  explicit Expression(const bus_size_t size) :
      Bus(size) {};
};

/// The 'arg' expression. (Direct association such as _l_10 = z or ba = 0110)
class ArgExpression : public Expression {
  static constexpr Kind KIND = Kind::ARG;
public:
  /// Constructor of a Arg Expression. This expression can be used to manually set
  /// a value to a variable. Example : _l_10 = 101.
  /// Beware a variable can be passed as an argument. It correspond to the case : _e = _l_5.
  /// \param arg Argument of the expression
  explicit ArgExpression(const Argument::ptr &arg) : Expression(arg->get_bus_size()), m_value(arg) {}

  /// ArgExpression kind : Kind::ARG
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the argument of this expression.
  [[nodiscard]] const Argument::ptr &get_argument() const noexcept { return m_value; }

  delete_copy_ctr(ArgExpression)

private:
  const Argument::ptr m_value;
};

/// The 'REG variable' expression.
class RegExpression : public Expression {
  static constexpr Kind KIND = Kind::REG;
public:
  /// Constructor of a Register operation. This operation can be used
  /// to get the last cycle value of a variable.
  /// \param var Variable identifier used
  explicit RegExpression(const Variable::ptr &var) : Expression(var->get_bus_size()), m_var(var) {}

  /// RegExpression kind : Kind::REG
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value to to which the operator applies.
  [[nodiscard]] const Variable::ptr &get_variable() const noexcept { return m_var; }

  delete_copy_ctr(RegExpression)

private:
  const Variable::ptr m_var;
};

/// The 'NOT arg' expression.
class NotExpression : public Expression {
  static constexpr Kind KIND = Kind::NOT;
public:
  explicit NotExpression(const Argument::ptr &arg) : Expression(arg->get_bus_size()), m_arg(arg) {}

  /// NotExpression Kind : Kind::NOT
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value to to which the operator applies.
  [[nodiscard]] const Argument::ptr &get_argument() const { return m_arg; }

  delete_copy_ctr(NotExpression)

private:
  const Argument::ptr m_arg;
};

/// The 'binop lhs rhs' expression.
class BinOpExpression : public Expression {
  static constexpr Kind KIND = Kind::BINOP;
public:
  enum class BinOpKind {
    OR, XOR, AND, NAND
  };

  /// Constructor of a Binary Expression
  /// \param binop Type of operation performed : AND, OR, etc.
  /// \param lhs Left hand side of the operation
  /// \param rhs Right hand side of the operation
  explicit BinOpExpression(BinOpKind binop, const Argument::ptr &lhs, const Argument::ptr &rhs);

  /// BinOpExpression Kind : Kind::BINOP
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Get the Binary Operation Kind : OR, AND, etc.
  [[nodiscard]] BinOpKind get_binop_kind() const noexcept { return m_kind; }

  /// Returns the left value to to which the operator applies.
  [[nodiscard]] const Argument::ptr &get_lhs_argument() const noexcept { return m_lhs; }

  /// Returns the right value to to which the operator applies.
  [[nodiscard]] const Argument::ptr &get_rhs_argument() const noexcept { return m_rhs; }

  delete_copy_ctr(BinOpExpression)

private:
  const BinOpKind m_kind;
  const Argument::ptr m_lhs;
  const Argument::ptr m_rhs;
};

/// The 'MUX choice true_arg false_arg' expression.
class MuxExpression : public Expression {
  static constexpr Kind KIND = Kind::MUX;
public:
  /// Constructor of a MuxExpression
  /// \param choice Bit used to make a choice. Must have a bus size of 1
  /// \param true_arg Value of the expression if choice bit is 1
  /// \param false_arg Value of the expression if choice bit is 0
  explicit MuxExpression(const Argument::ptr &choice, const Argument::ptr &true_arg, const Argument::ptr &false_arg);

  /// MuxExpression Kind : Kind::MUX
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the value used to make a choice.
  [[nodiscard]] const Argument::ptr &get_choice_argument() const noexcept { return m_choice; }

  /// Returns the value used if the choice bit is true.
  [[nodiscard]] const Argument::ptr &get_true_argument() const noexcept { return m_true; }

  /// Returns the value used if the choice bit is false.
  [[nodiscard]] const Argument::ptr &get_false_argument() const noexcept { return m_false; }

  delete_copy_ctr(MuxExpression)

private:
  const Argument::ptr m_choice;
  const Argument::ptr m_true;
  const Argument::ptr m_false;
};

/// The 'ROM addr_size word_size read_addr' expression.
class RomExpression : public Expression {
  static constexpr Kind KIND = Kind::ROM;
public:
  /// Constructor of a RomExpression
  /// \param addr_size The size of an address bus
  /// \param word_size The size of the word read
  /// \param read_addr The address to read to
  explicit RomExpression(bus_size_t addr_size, bus_size_t word_size, const Argument::ptr &read_addr);

  /// RomExpression Kind : Kind::ROM
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns read address
  [[nodiscard]] const Argument::ptr &get_read_address() const noexcept { return m_read_addr; }

  /// Returns the bus size of the read address
  [[nodiscard]] bus_size_t get_address_size() const noexcept { return m_read_addr->get_bus_size(); }

  delete_copy_ctr(RomExpression)

private:
  const Argument::ptr m_read_addr;
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
  explicit RamExpression(bus_size_t addr_size, bus_size_t word_size, const Argument::ptr &read_addr,
                         const Argument::ptr &write_enable, const Argument::ptr &write_addr, const Argument::ptr &data);

  /// RamExpression Kind : Kind::RAM
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the bus size of the read and write address
  [[nodiscard]] bus_size_t get_address_size() const noexcept { return m_read_addr->get_bus_size(); }

  /// Returns read address
  [[nodiscard]] const Argument::ptr &get_read_address() const noexcept { return m_read_addr; }

  /// Returns write enable bit flag
  [[nodiscard]] const Argument::ptr &get_write_enable() const noexcept { return m_write_enable; }

  /// Returns write address
  [[nodiscard]] const Argument::ptr &get_write_address() const noexcept { return m_write_addr; }

  /// Returns write data argument
  [[nodiscard]] const Argument::ptr &get_write_data() const noexcept { return m_write_data; }

  delete_copy_ctr(RamExpression)

private:
  const Argument::ptr m_read_addr;
  const Argument::ptr m_write_enable;
  const Argument::ptr m_write_addr;
  const Argument::ptr m_write_data;
};

/// The 'CONCAT begin_arg end_arg' expression.
class ConcatExpression : public Expression {
  static constexpr Kind KIND = Kind::CONCAT;
public:
  /// Constructor of the concatenation of two buses.
  /// \param begin_arg First part of the concatenation. It will be in the beginning of the created bus.
  /// \param end_arg Lst part of the concatenation. It will be at the end of the created bus.
  explicit ConcatExpression(const Argument::ptr &begin_arg, const Argument::ptr &end_arg);

  /// ConcatExpression Kind : Kind::CONCAT
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the beginning of the bus.
  [[nodiscard]] const Argument::ptr &get_beginning_part() const noexcept { return m_beg; }

  /// Returns the end of the bus.
  [[nodiscard]] const Argument::ptr &get_last_part() const noexcept { return m_last; }

  delete_copy_ctr(ConcatExpression)

private:
  const Argument::ptr m_beg;
  const Argument::ptr m_last;
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
  explicit SliceExpression(bus_size_t begin, bus_size_t end, const Argument::ptr &arg);

  /// SliceExpression Kind : Kind::SLICE
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the beginning index of the slice.
  [[nodiscard]]  bus_size_t get_begin_index() const noexcept { return m_begin; }

  /// Returns the ending index of the slice INCLUDED.
  [[nodiscard]] bus_size_t get_end_index() const noexcept { return m_end; }

  /// Returns the argument.
  [[nodiscard]] const Argument::ptr &get_argument() const noexcept { return m_arg; }

  delete_copy_ctr(SliceExpression)

private:
  const bus_size_t m_begin;
  const bus_size_t m_end;
  const Argument::ptr m_arg;
};

/// The 'SELECT index arg' expression.
class SelectExpression : public Expression {
  static constexpr Kind KIND = Kind::SELECT;
public:
  /// Constructor of the Select expression. Select the 'index'-th bit of the bus 'arg'.
  /// \param index Bit index to select
  /// \param arg Argument
  explicit SelectExpression(bus_size_t index, const Argument::ptr &arg);

  /// SelectExpression Kind : Kind::SELECT
  [[nodiscard]] Kind get_kind() const noexcept override { return KIND; }

  /// Returns the index of the bit selected.
  [[nodiscard]]  bus_size_t get_index() const noexcept { return m_index; }

  /// Returns the argument.
  [[nodiscard]] const Argument::ptr &get_argument() const noexcept { return m_arg; }

  delete_copy_ctr(SelectExpression)

private:
  const bus_size_t m_index;
  const Argument::ptr m_arg;
};

class Parser;

class Program {
public:
  typedef std::unique_ptr<Program> ptr;

  [[nodiscard]] const std::vector<Variable::ptr> &get_inputs() const noexcept { return m_input; };

  [[nodiscard]] const std::vector<Variable::ptr> &get_outputs() const noexcept { return m_output; };

  [[nodiscard]] const std::unordered_map<Variable::ptr, Expression::ptr> &get_equations() const noexcept {
    return m_eq;
  };

  delete_copy_ctr(Program);

private:
  friend Parser;
  // All the variables categorized as 'input'
  std::vector<Variable::ptr> m_input = {};

  // All the variables categorized as 'output'
  std::vector<Variable::ptr> m_output = {};

  // All equations
  std::unordered_map<Variable::ptr, Expression::ptr> m_eq = {};

  Program() = default;
};

template<typename Derived> class Visitor {
public:
#define DISPATCH(call) (static_cast<const Derived *>(this)->call)

  void visit(const Argument::ptr &arg) const {
    assert(arg != nullptr);

    switch (arg->get_kind()) {
    case Argument::Kind::CONSTANT:
      return DISPATCH(visit_constant(*dynamic_cast<const Constant *>(arg.get())));
    case Argument::Kind::VARIABLE:
      return DISPATCH(visit_variable(*dynamic_cast<const Variable *>(arg.get())));
    }
  }

  virtual void visit_argument(const Argument &arg) const {}
  virtual void visit_constant(const Constant &cst) const { return DISPATCH(visit_argument(cst)); }
  virtual void visit_variable(const Variable &var) const { return DISPATCH(visit_argument(var)); }

  void visit(const Expression::ptr &expr) const {
    assert(expr != nullptr);

    switch (expr->get_kind()) {
    case Expression::Kind::ARG:
      return DISPATCH(visit_arg_expr(*dynamic_cast<const ArgExpression *>(expr.get())));
    case Expression::Kind::REG:
      return DISPATCH(visit_reg_expr(*dynamic_cast<const RegExpression *>(expr.get())));
    case Expression::Kind::NOT:
      return DISPATCH(visit_not_expr(*dynamic_cast<const NotExpression *>(expr.get())));
    case Expression::Kind::BINOP:
      return DISPATCH(visit_binop_expr(*dynamic_cast<const BinOpExpression *>(expr.get())));
    case Expression::Kind::MUX:
      return DISPATCH(visit_mux_expr(*dynamic_cast<const MuxExpression *>(expr.get())));
    case Expression::Kind::ROM:
      return DISPATCH(visit_rom_expr(*dynamic_cast<const RomExpression *>(expr.get())));
    case Expression::Kind::RAM:
      return DISPATCH(visit_ram_expr(*dynamic_cast<const RamExpression *>(expr.get())));
    case Expression::Kind::CONCAT:
      return DISPATCH(visit_concat_expr(*dynamic_cast<const ConcatExpression *>(expr.get())));
    case Expression::Kind::SLICE:
      return DISPATCH(visit_slice_expr(*dynamic_cast<const SliceExpression *>(expr.get())));
    case Expression::Kind::SELECT:
      return DISPATCH(visit_select_expr(*dynamic_cast<const SelectExpression *>(expr.get())));
    }
  }

  virtual void visit_expr(const Expression &expr) const {}
  virtual void visit_arg_expr(const ArgExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_reg_expr(const RegExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_not_expr(const NotExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_binop_expr(const BinOpExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_mux_expr(const MuxExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_rom_expr(const RomExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_ram_expr(const RamExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_concat_expr(const ConcatExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_slice_expr(const SliceExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
  virtual void visit_select_expr(const SelectExpression &expr) const {
    return DISPATCH(visit_expr(expr));
  }
};

#endif // NETLIST_PROGRAM_HPP
