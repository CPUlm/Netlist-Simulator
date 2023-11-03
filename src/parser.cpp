#include "parser.hpp"
#include "utils.hpp"

#include <cassert>
#include <charconv>

/// Returns the radix of the given integer literal if explicitly written.
/// Otherwise returns 0.
[[nodiscard]] static unsigned get_integer_literal_radix(std::string_view literal) {
  if (literal.size() > 2) {
    switch (literal[1]) {
    case 'b':
    case 'B':
      return 2;
    case 'd':
    case 'D':
      return 10;
    case 'x':
    case 'X':
      return 16;
    }
  }

  return 0;
}

template <typename T>
[[nodiscard]] static T parse_integer_literal(std::string_view literal, unsigned default_radix = 2) {
  unsigned radix = get_integer_literal_radix(literal);
  if (radix > 0)
    literal.remove_prefix(2);
  else
    radix = default_radix;

  T value;
  // We must use data() and data() + size() instead of begin() and end() because
  // some standard implementations return special iterators (for debugging purposes) instead of raw pointers.
  const auto result = std::from_chars(literal.data(), literal.data() + literal.size(), value, radix);
  // The lexer have already checked the syntax.
  assert((result.ec == std::errc{}) && (result.ptr == (literal.data() + literal.size())));
  return value;
}

Parser::Parser(ReportManager &report_manager, Lexer &lexer) : m_report_manager(report_manager), m_lexer(lexer) {
  // Gets the first token
  m_lexer.tokenize(m_token);
}

std::shared_ptr<Program> Parser::parse_program() {
  parse_inputs();
  parse_outputs();
  parse_variables();
  parse_equations();
  return m_program_builder.build();
}

/// Grammar:
/// ```
/// opt-size-specifier := ":" INTEGER
///                     |
/// ```
std::optional<bus_size_t> Parser::parse_size_specifier() {
  if (m_token.kind != TokenKind::COLON)
    return std::nullopt;

  consume(); // eat COLON

  return parse_bus_size();
}

/// Grammar:
/// ```
/// variable-decl := IDENTIFIER opt-size-specifier
/// variable-decl-list := variable-decl
///                     | variable-decl-list "," variable-decl
///                     |
/// ```
void Parser::parse_variables_common(bool allow_size_specifier,
                                    const std::function<bool(SourceLocation, std::string_view, size_t)> &handler) {
  if (m_token.kind != TokenKind::IDENTIFIER)
    return; // allow an empty list

  do {
    if (m_token.kind != TokenKind::IDENTIFIER) {
      unexpected_token_error(m_token, "a comma");
    }

    SourceLocation variable_location = m_token.position;
    std::string_view variable_name = m_token.spelling;

    consume(); // eat IDENTIFIER

    std::optional<size_t> size_specifier;
    if (allow_size_specifier) {
      size_specifier = parse_size_specifier();
      if (!size_specifier.has_value())
        size_specifier = 1;
    } else {
      size_specifier = 0;
    }

    if (!handler(variable_location, variable_name, size_specifier.value()))
      return;

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat COMMA
      continue;
    } else {
      return;
    }
  } while (true);
}

/// Grammar:
/// ```
/// inputs := "INPUT" variable-decl-list
/// ```
void Parser::parse_inputs() {
  if (m_token.kind != TokenKind::KEY_INPUT) {
    unexpected_token_error(m_token, "the keyword `INPUT'");
  }

  consume(); // eat `INPUT`

  return parse_variables_common(
      /* allow_size_specifier= */ false,
      [this](SourceLocation variable_location, std::string_view variable_name, size_t size_in_bits) {
        auto it = m_variables.find(variable_name);
        if (it != m_variables.end()) {
          // The variable is already declared.
          m_report_manager.report(ReportSeverity::ERROR)
              .with_location(variable_location)
              .with_span({variable_location, (uint32_t)variable_name.size()})
              .with_message("the input `{}' is defined more than once", variable_name)
              .finish()
              .exit();
          return false;
        }

        const VariableInfo variable_info = {0, variable_location,
                                            /* is_input= */ true,
                                            /* is_output= */ false};
        m_variables.insert({variable_name, variable_info});
        return true;
      });
}

/// Grammar:
/// ```
/// outputs := "OUTPUT" variable-decl-list
/// ```
void Parser::parse_outputs() {
  if (m_token.kind != TokenKind::KEY_OUTPUT) {
    unexpected_token_error(m_token, "the keyword `OUTPUT'");
  }

  consume(); // eat `OUTPUT`

  return parse_variables_common(
      /* allow_size_specifier= */ false,
      [this](SourceLocation variable_location, std::string_view variable_name, size_t size_in_bits) {
        auto it = m_variables.find(variable_name);
        if (it != m_variables.end()) {
          // The variable is already declared.
          auto builder = m_report_manager.report(ReportSeverity::ERROR);
          builder.with_location(variable_location).with_span({variable_location, (uint32_t)variable_name.size()});

          if (it->second.is_input) {
            builder.with_message("the variable `{}' is defined both as input and output", variable_name);
          } else {
            builder.with_message("the output `{}' is defined more than once", variable_name);
          }

          builder.finish().exit();
          return false;
        }

        const VariableInfo variable_info = {0, variable_location,
                                            /* is_input= */ false,
                                            /* is_output= */ true};
        m_variables.insert({variable_name, variable_info});
        return true;
      });
}

/// Grammar:
/// ```
/// variables := "VAR" variable-decl-list
/// ```
void Parser::parse_variables() {
  if (m_token.kind != TokenKind::KEY_VAR) {
    unexpected_token_error(m_token, "the keyword `VAR'");
  }

  consume(); // eat `VAR`

  std::unordered_set<std::string_view> already_defined_variables;
  return parse_variables_common(
      /* allow_size_specifier= */ true,
      [this, &already_defined_variables](SourceLocation variable_location, std::string_view variable_name,
                                         size_t size_in_bits) {
        auto it = m_variables.find(variable_name);
        if (it != m_variables.end()) {
          if (already_defined_variables.contains(variable_name)) {
            m_report_manager.report(ReportSeverity::ERROR)
                .with_location(variable_location)
                .with_span({variable_location, (uint32_t)variable_name.size()})
                .with_message("the variable `{}' is defined more than once", variable_name)
                .finish()
                .exit();
            return false;
          }

          unsigned flags = RIF_NONE;
          if (it->second.is_input)
            flags |= RIF_INPUT;
          if (it->second.is_output)
            flags |= RIF_OUTPUT;
          already_defined_variables.insert(variable_name);
          it->second.reg = m_program_builder.add_register(size_in_bits, std::string{variable_name}, flags);
          return true;
        }

        const reg_t reg = m_program_builder.add_register(size_in_bits, std::string{variable_name});
        const VariableInfo variable_info = {reg, variable_location,
                                            /* is_input= */ false,
                                            /* is_output= */ false};
        m_variables.insert({variable_name, variable_info});
        already_defined_variables.insert(variable_name);
        return true;
      });
}

/// Grammar:
/// ```
/// equations := "IN" equation-list
/// equation-list := equation
///                | equation-list equation
/// ```
void Parser::parse_equations() {
  // We expect the `IN` keyword, but if it's absent we just pretend it's there.
  if (m_token.kind != TokenKind::KEY_IN) {
    unexpected_token_error(m_token, "the keyword `IN'");
  } else {
    consume(); // eat `IN`
  }

  while (m_token.kind != TokenKind::EOI)
    parse_equation();
}

/// Grammar:
/// ```
/// equation := IDENTIFIER "=" expression
/// ```
void Parser::parse_equation() {
  if (m_token.kind != TokenKind::IDENTIFIER)
    unexpected_token_error(m_token, "an equation label");

  std::string_view variable_label = m_token.spelling;
  auto it = m_variables.find(variable_label);
  if (it == m_variables.end()) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)variable_label.size()})
        .with_message("equation label `{}' not declared inside `VAR' declaration", variable_label)
        .finish()
        .exit();
  } else if (it->second.is_input) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)variable_label.size()})
        .with_message("cannot assign an expression to the input variable `{}'", variable_label)
        .finish()
        .exit();
  }

  consume(); // eat IDENTIFIER

  if (m_token.kind != TokenKind::EQUAL)
    unexpected_token_error(m_token, "a `=' followed by an expression`");

  consume(); // eat `=`

  reg_t output_reg = it->second.reg;
  parse_expression(output_reg);
}

/// Grammar:
/// ```
/// expression := const-expression
///             | load-expression
///             | not-expression
///             | reg-expression
///             | binary-expression
///             | mux-expression
///             | concat-expression
///             | select-expression
///             | slice-expression
///             | ram-expression
///             | rom-expression
/// ```
void Parser::parse_expression(reg_t output) {
  switch (m_token.kind) {
  case TokenKind::INTEGER:
    return parse_const_expression(output);
  case TokenKind::IDENTIFIER:
    return parse_load_expression(output);
  case TokenKind::KEY_NOT:
    return parse_not_expression(output);
  case TokenKind::KEY_REG:
    return parse_reg_expression(output);
  case TokenKind::KEY_AND:
  case TokenKind::KEY_NAND:
  case TokenKind::KEY_OR:
  case TokenKind::KEY_NOR:
  case TokenKind::KEY_XOR:
  case TokenKind::KEY_XNOR:
    return parse_binary_expression(output);
  case TokenKind::KEY_MUX:
    return parse_mux_expression(output);
  case TokenKind::KEY_CONCAT:
    return parse_concat_expression(output);
  case TokenKind::KEY_SELECT:
    return parse_select_expression(output);
  case TokenKind::KEY_SLICE:
    return parse_slice_expression(output);
  case TokenKind::KEY_RAM:
    return parse_ram_expression(output);
  case TokenKind::KEY_ROM:
    return parse_rom_expression(output);
  default:
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
        .with_message("invalid expression, expected an operator or a constant")
        .finish()
        .exit();
  }
}

[[nodiscard]] static std::optional<bus_size_t> get_bus_size_of_constant(std::string_view literal) {
  auto radix = get_integer_literal_radix(literal);
  if (radix > 0)
    literal.remove_prefix(2);
  else
    radix = 2;

  switch (radix) {
  case 2:
    // TODO: check for integer size
    return static_cast<bus_size_t>(literal.size());
  case 16:
    return static_cast<bus_size_t>(literal.size() * 4);
  default:
    return std::nullopt;
  }
}

/// Grammar:
/// ```
/// constant := INTEGER <opt-size-specifier>
/// ```
std::pair<reg_value_t, bus_size_t> Parser::parse_constant() {
  assert(m_token.kind == TokenKind::INTEGER);

  unsigned radix = get_integer_literal_radix(m_token.spelling);
  if (radix == 0)
    radix = 2;

  check_invalid_digits(m_token, radix);
  const auto integer_token = m_token;
  consume(); // eat INTEGER

  auto bus_size = parse_size_specifier();
  if (!bus_size.has_value()) {
    bus_size = get_bus_size_of_constant(integer_token.spelling);

    if (!bus_size.has_value()) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(integer_token.position)
          .with_span({integer_token.position, (std::uint_least32_t)integer_token.spelling.size()})
          .with_message("a decimal integer constant needs a explicit bus size")
          .finish()
          .exit();
    }
  }

  assert(bus_size.has_value());
  const auto value = parse_integer_literal<reg_value_t>(integer_token.spelling);
  return {value, bus_size.value()};
}

/// Grammar:
/// ```
/// bus-size := INTEGER (without any radix prefix, in decimal)
/// ```
bus_size_t Parser::parse_bus_size(bool as_index) {
  if (m_token.kind != TokenKind::INTEGER)
    unexpected_token_error(m_token, "a decimal integer constant");

  unsigned radix = get_integer_literal_radix(m_token.spelling);
  const bool has_prefix = radix > 0;
  if (radix == 0)
    radix = 10;

  check_invalid_digits(m_token, radix);
  const auto value = parse_integer_literal<bus_size_t>(m_token.spelling, radix);
  if (has_prefix) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (std::uint_least32_t)m_token.spelling.size()})
        .with_message("explicit radix forbidden for bus size constants")
        .with_note("write `{}' instead", value)
        .finish()
        .exit();
  }

  // Check if the bus size is valid:
  if (!as_index) {
    if (value > MAX_VARIABLE_SIZE) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
          .with_message("but size greater than {} bits is not allowed", MAX_VARIABLE_SIZE)
          .finish()
          .exit();
    } else if (value == 0) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
          .with_message("but size 0 is not allowed")
          .finish()
          .exit();
    }
  }

  consume(); // eat INTEGER
  return value;
}

void Parser::check_invalid_digits(Token &token, unsigned int radix) {
  std::string_view spelling = token.spelling;
  bool has_prefix = get_integer_literal_radix(spelling) > 0;
  if (has_prefix)
    spelling.remove_prefix(2);

  for (uint32_t i = 0; i < spelling.size(); ++i) {
    const char ch = spelling[i];
    bool invalid_digit = false;
    switch (radix) {
    case 2:
      invalid_digit = !is_bin_digit(ch);
      break;
    case 10:
      invalid_digit = !is_digit(ch);
      break;
    case 16:
      invalid_digit = !is_hex_digit(ch);
      break;
    default:
      assert(false && "unreachable");
    }

    if (invalid_digit) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location({token.position.offset + i})
          .with_span({{token.position.offset + i}, 1})
          .with_message("invalid digit in the constant")
          .with_note("the radix of the constant is {}", radix)
          .finish()
          .exit();
    }
  }
}

/// Grammar:
/// ```
/// register := IDENTIFIER
/// ```
[[nodiscard]] reg_t Parser::parse_register() {
  if (m_token.kind != TokenKind::IDENTIFIER)
    unexpected_token_error(m_token, "a register");

  const auto it = m_variables.find(m_token.spelling);
  if (it == m_variables.end()) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
        .with_message("variable `{}' not found", m_token.spelling)
        .finish()
        .exit();
  }

  consume(); // eat IDENTIFIER
  return it->second.reg;
}

/// Grammar:
/// ```
/// arg := <register>
///      | <constant>
/// ```
reg_t Parser::parse_argument() {
  switch (m_token.kind) {
  case TokenKind::IDENTIFIER:
    return parse_register();
  case TokenKind::INTEGER: {
    // For the following code: output = AND a 0110
    // We generate something like that:
    // _temp_0 = CONST 0110
    // output = AND a _temp_0

    const auto [value, bus_size] = parse_constant();
    const reg_t reg = m_program_builder.add_register(bus_size);
    m_program_builder.add_const(reg, value);
    return reg;
  }
  default:
    unexpected_token_error(m_token, "a variable or a constant");
  }
}

/// Grammar:
/// ```
/// const-expression := <constant>
/// ```
void Parser::parse_const_expression(reg_t output) {
  assert(m_token.kind == TokenKind::INTEGER);
  const auto [value, bus_size] = parse_constant();
  m_program_builder.add_const(output, value);
}

/// Grammar:
/// ```
/// load-expression := <register>
/// ```
void Parser::parse_load_expression(reg_t output) {
  assert(m_token.kind == TokenKind::IDENTIFIER);
  const auto input = parse_register();
  m_program_builder.add_load(output, input);
}

/// Grammar:
/// ```
/// not-expression := "NOT" <arg>
/// ```
void Parser::parse_not_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_NOT);
  consume(); // eat `NOT`

  auto input = parse_argument();
  m_program_builder.add_not(output, input);
}

/// Grammar:
/// ```
/// reg-expression := "REG" <register>
/// ```
void Parser::parse_reg_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_REG);
  consume(); // eat `REG`

  auto input = parse_register();
  m_program_builder.add_reg(output, input);
}

/// Grammar:
/// ```
/// binary-expression := binary-opcode <arg> <arg>
///
/// binary-opcode := "AND"
///                | "NAND"
///                | "OR"
///                | "NOR"
///                | "XOR"
///                | "XNOR"
/// ```
void Parser::parse_binary_expression(reg_t output) {
  auto token_kind = m_token.kind;
  consume(); // eat the binary operator keyword

  auto lhs_reg = parse_argument();
  auto rhs_reg = parse_argument();

  switch (token_kind) {
  case TokenKind::KEY_AND:
    m_program_builder.add_and(output, lhs_reg, rhs_reg);
    break;
  case TokenKind::KEY_NAND:
    m_program_builder.add_nand(output, lhs_reg, rhs_reg);
    break;
  case TokenKind::KEY_OR:
    m_program_builder.add_or(output, lhs_reg, rhs_reg);
    break;
  case TokenKind::KEY_NOR:
    m_program_builder.add_nor(output, lhs_reg, rhs_reg);
    break;
  case TokenKind::KEY_XOR:
    m_program_builder.add_xor(output, lhs_reg, rhs_reg);
    break;
  case TokenKind::KEY_XNOR:
    m_program_builder.add_xnor(output, lhs_reg, rhs_reg);
    break;
  default:
    assert(false && "unreachable code");
    break;
  }
}

/// Grammar:
/// ```
/// mux-expression := "MUX" <arg> <arg> <arg>
/// ```
void Parser::parse_mux_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_MUX);
  consume(); // eat `MUX`

  auto choice = parse_argument();
  auto first = parse_argument();
  auto second = parse_argument();
  m_program_builder.add_mux(output, choice, first, second);
}

/// Grammar:
/// ```
/// concat-expression := "CONCAT" <arg> <arg>
/// ```
void Parser::parse_concat_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_CONCAT);
  consume(); // eat `CONCAT`

  auto lhs = parse_argument();
  auto rhs = parse_argument();
  m_program_builder.add_concat(output, lhs, rhs);
}

/// Grammar:
/// ```
/// slice-expression := "SELECT" <bus-size> <arg>
/// ```
void Parser::parse_select_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_SELECT);

  consume(); // eat `SELECT`

  const auto i = parse_bus_size(/*as_index=*/true);
  const auto input = parse_argument();

  m_program_builder.add_select(output, i, input);
}

/// Grammar:
/// ```
/// slice-expression := "SLICE" <bus-size> <bus-size> <arg>
/// ```
void Parser::parse_slice_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_SLICE);

  consume(); // eat `SLICE`

  const auto start = parse_bus_size(/*as_index=*/true);
  const auto end = parse_bus_size(/*as_index=*/true);
  const auto input = parse_argument();

  m_program_builder.add_slice(output, start, end, input);
}

/// Grammar:
/// ```
/// ram-expression := "RAM" <bus-size> <bus-size> <arg>
/// ```
void Parser::parse_rom_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_ROM);

  consume(); // eat `ROM`

  const auto addr_size = parse_bus_size();
  const auto word_size = parse_bus_size();
  const auto read_addr = parse_argument();

  m_program_builder.add_rom(output, addr_size, word_size, read_addr);
}

/// Grammar:
/// ```
/// ram-expression := "RAM" <bus-size> <bus-size> <arg> <arg> <arg> <arg>
/// ```
void Parser::parse_ram_expression(reg_t output) {
  assert(m_token.kind == TokenKind::KEY_RAM);

  consume(); // eat `RAM`

  const auto addr_size = parse_bus_size();
  const auto word_size = parse_bus_size();
  const auto read_addr = parse_argument();
  const auto write_enable = parse_argument();
  const auto write_addr = parse_argument();
  const auto write_data = parse_argument();

  m_program_builder.add_ram(output, addr_size, word_size, read_addr, write_enable, write_addr, write_data);
}

void Parser::consume() {
  m_lexer.tokenize(m_token);
}

void Parser::unexpected_token_error(const Token &token, std::string_view expected_token_name) {
  m_report_manager.report(ReportSeverity::ERROR)
      .with_location(token.position)
      .with_span({token.position, (uint32_t)token.spelling.size()})
      .with_message("unexpected token; expected {}", expected_token_name)
      .finish()
      .exit();
}
