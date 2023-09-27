#ifndef NETLIST_PROGRAM_HPP
#define NETLIST_PROGRAM_HPP

#include <cassert>
#include <memory>
#include <string_view>
#include <vector>

class Expression;

/// The base class for values.
///
/// There is three kinds of value: constants, inputs and equations.
class Value {
public:
  enum class Kind {
    /// An instance of the class Constant.
    CONSTANT,
    /// An instance of the class Input.
    INPUT,
    /// An instance of the class Equation.
    EQUATION
  };

  [[nodiscard]] virtual Kind get_kind() const = 0;

  /// Returns true if it is an instance of Input.
  [[nodiscard]] bool is_input() const { return get_kind() == Kind::INPUT; }
  /// Returns true if it is an instance of Constant.
  [[nodiscard]] bool is_constant() const {
    return get_kind() == Kind::CONSTANT;
  }
  /// Returns true if it is an instance of Equation.
  [[nodiscard]] bool is_equation() const {
    return get_kind() == Kind::EQUATION;
  }

  /// Returns true if this value is named, that is its name is non empty.
  [[nodiscard]] bool is_named() const { return !m_name.empty(); }
  /// Returns the name either of the input or the equation. If the value
  /// is a constant then it doesn't have any name and therefore an empty
  /// string is returned.
  [[nodiscard]] std::string_view get_name() const { return m_name; }

  /// Returns the size, in bits, of the underlying bus.
  [[nodiscard]] size_t get_size_in_bits() const { return m_size_in_bits; }

protected:
  explicit Value(std::string_view name = {}) : m_name(name) {}

private:
  std::string_view m_name;
  size_t m_size_in_bits = 1;
};

/// Represents a constant in the source code such as `t` or `42`.
class Constant : public Value {
public:
  static constexpr Kind KIND = Kind::CONSTANT;
  [[nodiscard]] Kind get_kind() const override { return KIND; }

  explicit Constant(size_t value) noexcept : Value(), m_value(value) {}

  /// Returns true if this constant represents the false constant.
  [[nodiscard]] bool is_false() const { return m_value == 0; }
  /// Returns true if this constant represents the true constant.
  /// Note that unlike in C we only consider constant value equal to 1
  /// to meaning true.
  [[nodiscard]] bool is_true() const { return m_value == 1; }

  /// Returns the value of the constant.
  [[nodiscard]] size_t get_value() const { return m_value; }

private:
  // TODO(hgruniaux): For now, integer values are represented using size_t
  //                  which is either 32-bits or 64-bits. If we want to work
  //                  with buses spanning more than 64-bits then we will have
  //                  a problem. Maybe use an arbitrary-precision integer
  //                  library (GMP for example) to encode values.
  size_t m_value = 0;
};

/// Represents an input variable as specified in the `INPUT` statement.
class Input : public Value {
public:
  static constexpr Kind KIND = Kind::INPUT;
  [[nodiscard]] Kind get_kind() const override { return KIND; }

  explicit Input(std::string_view name) noexcept : Value(name) {}
};

/// Represents an equation linking a variable and an expression.
class Equation : public Value {
public:
  static constexpr Kind KIND = Kind::EQUATION;
  [[nodiscard]] Kind get_kind() const override { return KIND; }

  explicit Equation(std::string_view name, Expression *expr = nullptr) noexcept
      : Value(name), m_expr(expr) {}

  /// Returns true if this equation is marked as an output (variable name
  /// declared inside the `OUTPUT` statement).
  [[nodiscard]] bool is_output() const { return m_is_output; }
  /// Marks this equation as assigning a value to an output.
  void mark_as_output() { m_is_output = true; }

  /// The right-hand-side expression of the equation.
  [[nodiscard]] Expression *get_expression() { return m_expr; }
  [[nodiscard]] const Expression *get_expression() const { return m_expr; }
  void set_expression(Expression *expr) { m_expr = expr; }

private:
  Expression *m_expr = nullptr;
  bool m_is_output = false;
};

/// The base class to all expressions.
class Expression {
public:
  enum class Kind { NOT, BINARY };

  [[nodiscard]] virtual Kind get_kind() const = 0;
};

/// The `NOT value` expression.
class NotExpression : public Expression {
public:
  static constexpr Kind KIND = Kind::NOT;
  [[nodiscard]] Kind get_kind() const override { return KIND; }

  explicit NotExpression(Value *value) noexcept
      : Expression(), m_value(value) {}

  /// Returns the value to to which the operator applies.
  [[nodiscard]] Value *get_value() { return m_value; }
  [[nodiscard]] const Value *get_value() const { return m_value; }

private:
  Value *m_value = nullptr;
};

/// The different supported binary operators.
enum class BinaryOp { AND, OR, NAND, XOR };

/// A binary expression such as `AND lhs rhs`.
class BinaryExpression : public Expression {
public:
  static constexpr Kind KIND = Kind::BINARY;
  [[nodiscard]] Kind get_kind() const override { return KIND; }

  BinaryExpression(BinaryOp op, Value *lhs, Value *rhs) noexcept
      : Expression(), m_binop(op), m_lhs(lhs), m_rhs(rhs) {}

  /// Returns the binary operator of this expression.
  [[nodiscard]] BinaryOp get_operator() const { return m_binop; }

  /// Returns the left-hand-side of the expression.
  [[nodiscard]] Value *get_lhs() { return m_lhs; }
  [[nodiscard]] const Value *get_lhs() const { return m_lhs; }

  /// Returns the right-hand-side of the expression.
  [[nodiscard]] Value *get_rhs() { return m_rhs; }
  [[nodiscard]] const Value *get_rhs() const { return m_rhs; }

private:
  BinaryOp m_binop = BinaryOp::AND;
  Value *m_lhs = nullptr;
  Value *m_rhs = nullptr;
};

class Program {
public:
  /// Returns the name of all registered inputs.
  [[nodiscard]] std::vector<std::string_view> get_input_names() const;
  /// Returns the name of all registered outputs.
  [[nodiscard]] std::vector<std::string_view> get_output_names() const;

  /// Returns all the values of the program, including the inputs,
  /// the constants and the equations (which also include the outputs).
  [[nodiscard]] const std::vector<std::unique_ptr<Value>> &get_values() const {
    return m_values;
  }

  /// Returns the program's equations.
  [[nodiscard]] const std::vector<Equation *> &get_equations() const {
    return m_equations;
  }

  // The different factory functions to build the parts of the program.
  // They are mainly called by the parser.

  [[nodiscard]] Constant *create_constant(size_t value);
  [[nodiscard]] Input *create_input(std::string_view name);
  [[nodiscard]] Equation *create_output(std::string_view name);
  [[nodiscard]] Equation *create_equation(std::string_view name);

  [[nodiscard]] NotExpression *create_not_expr(Value *value);
  [[nodiscard]] BinaryExpression *create_binary_expr(BinaryOp op, Value *lhs,
                                                     Value *rhs);

private:
  std::vector<std::unique_ptr<Value>> m_values;
  std::vector<std::unique_ptr<Expression>> m_expressions;
  std::vector<Input *> m_inputs;
  std::vector<Equation *> m_equations;
};

template <typename Derived> class Visitor {
public:
#define DISPATCH(call) (static_cast<Derived *>(this)->call)

  void visit(Value *value) {
    assert(value != nullptr);

    switch (value->get_kind()) {
    case Value::Kind::CONSTANT:
      return DISPATCH(visit_constant(static_cast<Constant *>(value)));
    case Value::Kind::INPUT:
      return DISPATCH(visit_input(static_cast<Input *>(value)));
    case Value::Kind::EQUATION:
      return DISPATCH(visit_equation(static_cast<Equation *>(value)));
    }
  }

  void visit_value(Value *value) {}
  void visit_constant(Constant *value) { return DISPATCH(visit_value(value)); }
  void visit_input(Input *value) { return DISPATCH(visit_value(value)); }
  void visit_equation(Equation *value) { return DISPATCH(visit_value(value)); }

  void visit(Expression *expr) {
    assert(expr != nullptr);

    switch (expr->get_kind()) {
    case Expression::Kind::NOT:
      return DISPATCH(visit_not_expr(static_cast<NotExpression *>(expr)));
    case Expression::Kind::BINARY:
      return DISPATCH(visit_binary_expr(static_cast<BinaryExpression *>(expr)));
    }
  }

  void visit_expr(Expression *expr) {}
  void visit_not_expr(NotExpression *expr) {
    return DISPATCH(visit_expr(expr));
  }
  void visit_binary_expr(BinaryExpression *expr) {
    return DISPATCH(visit_expr(expr));
  }
};

#endif // NETLIST_PROGRAM_HPP
