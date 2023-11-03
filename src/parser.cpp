#include "parser.hpp"

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
  if (!(parse_inputs() && parse_outputs() && parse_variables() && parse_equations()))
    return nullptr;
  return m_program_builder.build();
}

/// Grammar:
/// ```
/// opt-size-specifier := ":" INTEGER
///                     | <no-size-specifier>
/// ```
std::optional<bus_size_t> Parser::parse_size_specifier() {
  if (m_token.kind != TokenKind::COLON)
    return std::nullopt;

  m_lexer.set_default_radix(10);

  consume(); // eat COLON and tokenize the eventual integer literal

  m_lexer.set_default_radix(2);

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "a bus size");
    return std::nullopt;
  }

  const auto size_in_bits = parse_integer_literal<bus_size_t>(m_token.spelling, /* default_radix= */ 10);

  const bool has_explicit_radix = get_integer_literal_radix(m_token.spelling) > 0;
  if (has_explicit_radix) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
        .with_message("explicit radix forbidden for bus size constants")
        .with_note("write `{}' instead", size_in_bits)
        .finish()
        .exit();
  }

  // Check if the size in bits is valid:
  if (size_in_bits > MAX_VARIABLE_SIZE) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
        .with_message("variables of bit size greater than {} bits is not allowed", MAX_VARIABLE_SIZE)
        .finish()
        .exit();
    return std::nullopt;
  } else if (size_in_bits == 0) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
        .with_message("variables of bit size 0 is not allowed")
        .finish()
        .exit();
    return std::nullopt;
  }

  consume(); // eat INTEGER
  return size_in_bits;
}

/// Grammar:
/// ```
/// variable-decl := IDENTIFIER opt-size-specifier
/// variable-decl-list := variable-decl
///                     | variable-decl-list "," variable-decl
///                     |
/// ```
bool Parser::parse_variables_common(bool allow_size_specifier,
                                    const std::function<bool(SourceLocation, std::string_view, size_t)> &handler) {
  if (m_token.kind != TokenKind::IDENTIFIER)
    return true; // allow empty list

  do {
    if (m_token.kind != TokenKind::IDENTIFIER) {
      emit_unexpected_token_error(m_token, "a comma");
      return false;
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
      return false;

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat COMMA
      continue;
    } else {
      return true;
    }
  } while (true);
}

/// Grammar:
/// ```
/// inputs := "INPUT" variable-decl-list
/// ```
bool Parser::parse_inputs() {
  if (m_token.kind != TokenKind::KEY_INPUT) {
    emit_unexpected_token_error(m_token, "the keyword `INPUT'");
    return false;
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
bool Parser::parse_outputs() {
  if (m_token.kind != TokenKind::KEY_OUTPUT) {
    emit_unexpected_token_error(m_token, "the keyword `OUTPUT'");
    return false;
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
bool Parser::parse_variables() {
  if (m_token.kind != TokenKind::KEY_VAR) {
    emit_unexpected_token_error(m_token, "the keyword `VAR'");
    return false;
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
bool Parser::parse_equations() {
  // We expect the `IN` keyword, but if it's absent we just pretend it's there.
  if (m_token.kind != TokenKind::KEY_IN) {
    emit_unexpected_token_error(m_token, "the keyword `IN'");
  } else {
    consume(); // eat `IN`
  }

  bool ok = true;
  while (m_token.kind != TokenKind::EOI && (ok &= parse_equation()))
    ;

  return ok;
}

/// Grammar:
/// ```
/// equation := IDENTIFIER "=" expression
/// ```
bool Parser::parse_equation() {
  if (m_token.kind != TokenKind::IDENTIFIER) {
    emit_unexpected_token_error(m_token, "an equation label");
    return false;
  }

  std::string_view variable_label = m_token.spelling;
  auto it = m_variables.find(variable_label);
  if (it == m_variables.end()) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)variable_label.size()})
        .with_message("equation label `{}' not declared inside `VAR' declaration", variable_label)
        .finish()
        .exit();
    return false;
  } else if (it->second.is_input) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)variable_label.size()})
        .with_message("cannot assign an expression to the input variable `{}'", variable_label)
        .finish()
        .exit();
    return false;
  }

  consume(); // eat IDENTIFIER

  if (m_token.kind != TokenKind::EQUAL) {
    emit_unexpected_token_error(m_token, "a `=' followed by an expression`");
    return false;
  }

  consume(); // eat `=`

  reg_t output_reg = it->second.reg;
  return parse_expression(output_reg);
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
bool Parser::parse_expression(reg_t output_reg) {
  switch (m_token.kind) {
  case TokenKind::INTEGER:
    return parse_const_expression(output_reg);
  case TokenKind::IDENTIFIER:
    return parse_load_expression(output_reg);
  case TokenKind::KEY_NOT:
    return parse_not_expression(output_reg);
  case TokenKind::KEY_REG:
    return parse_reg_expression(output_reg);
  case TokenKind::KEY_AND:
  case TokenKind::KEY_NAND:
  case TokenKind::KEY_OR:
  case TokenKind::KEY_NOR:
  case TokenKind::KEY_XOR:
  case TokenKind::KEY_XNOR:
    return parse_binary_expression(output_reg);
  case TokenKind::KEY_MUX:
    return parse_mux_expression(output_reg);
  case TokenKind::KEY_CONCAT:
    return parse_concat_expression(output_reg);
  case TokenKind::KEY_SELECT:
    return parse_select_expression(output_reg);
  case TokenKind::KEY_SLICE:
    return parse_slice_expression(output_reg);
  case TokenKind::KEY_RAM:
    return parse_ram_expression(output_reg);
  case TokenKind::KEY_ROM:
    return parse_rom_expression(output_reg);
  default:
    m_report_manager.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
        .with_message("invalid expression, expected an operator or a constant")
        .finish()
        .exit();
    return false;
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
/// argument := IDENTIFIER
///           | <constant>
/// ```
std::optional<reg_t> Parser::parse_argument() {
  switch (m_token.kind) {
  case TokenKind::IDENTIFIER: {
    const auto it = m_variables.find(m_token.spelling);
    if (it == m_variables.end()) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_span({m_token.position, (uint32_t)m_token.spelling.size()})
          .with_message("variable `{}' not found", m_token.spelling)
          .finish()
          .exit();
      return std::nullopt;
    }

    consume(); // eat IDENTIFIER
    return it->second.reg;
  }
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
    emit_unexpected_token_error(m_token, "a variable or a constant");
    return std::nullopt;
  }
}

/// Grammar:
/// ```
/// const-expression := <constant>
/// ```
bool Parser::parse_const_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::INTEGER);

  const auto [value, bus_size] = parse_constant();
  m_program_builder.add_const(output_reg, value);
  return true;
}

/// Grammar:
/// ```
/// load-expression := <register>
/// ```
bool Parser::parse_load_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::IDENTIFIER);

  const auto input_reg = parse_argument();
  if (!input_reg.has_value())
    return false;

  m_program_builder.add_load(output_reg, input_reg.value());
  return true;
}

/// Grammar:
/// ```
/// not-expression := "NOT" register
/// ```
bool Parser::parse_not_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_NOT);
  consume(); // eat `NOT`

  auto input_reg = parse_argument();
  if (!input_reg.has_value())
    return false;

  m_program_builder.add_not(output_reg, input_reg.value());
  return true;
}

/// Grammar:
/// ```
/// reg-expression := "REG" register
/// ```
bool Parser::parse_reg_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_REG);
  consume(); // eat `REG`

  auto input_reg = parse_argument();
  if (!input_reg.has_value())
    return false;

  m_program_builder.add_reg(output_reg, input_reg.value());
  return true;
}

/// Grammar:
/// ```
/// binary-expression := binary-opcode register register
///
/// binary-opcode := "AND"
///                | "NAND"
///                | "OR"
///                | "NOR"
///                | "XOR"
///                | "XNOR"
/// ```
bool Parser::parse_binary_expression(reg_t output_reg) {
  auto token_kind = m_token.kind;
  consume(); // eat the binary operator keyword

  auto lhs_reg = parse_argument();
  if (!lhs_reg.has_value())
    return false;

  auto rhs_reg = parse_argument();
  if (!rhs_reg.has_value())
    return false;

  switch (token_kind) {
  case TokenKind::KEY_AND:
    m_program_builder.add_and(output_reg, lhs_reg.value(), rhs_reg.value());
    break;
  case TokenKind::KEY_NAND:
    m_program_builder.add_nand(output_reg, lhs_reg.value(), rhs_reg.value());
    break;
  case TokenKind::KEY_OR:
    m_program_builder.add_or(output_reg, lhs_reg.value(), rhs_reg.value());
    break;
  case TokenKind::KEY_NOR:
    m_program_builder.add_nor(output_reg, lhs_reg.value(), rhs_reg.value());
    break;
  case TokenKind::KEY_XOR:
    m_program_builder.add_xor(output_reg, lhs_reg.value(), rhs_reg.value());
    break;
  case TokenKind::KEY_XNOR:
    m_program_builder.add_xnor(output_reg, lhs_reg.value(), rhs_reg.value());
    break;
  default:
    assert(false && "unreachable code");
    break;
  }

  return true;
}

/// Grammar:
/// ```
/// mux-expression := "MUX" register register register
/// ```
bool Parser::parse_mux_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_MUX);
  consume(); // eat `MUX`

  auto choice_reg = parse_argument();
  if (!choice_reg.has_value())
    return false;

  auto first_reg = parse_argument();
  if (!first_reg.has_value())
    return false;

  auto second_reg = parse_argument();
  if (!second_reg.has_value())
    return false;

  m_program_builder.add_mux(output_reg, choice_reg.value(), first_reg.value(), second_reg.value());
  return true;
}

/// Grammar:
/// ```
/// concat-expression := "CONCAT" register register
/// ```
bool Parser::parse_concat_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_CONCAT);
  consume(); // eat `CONCAT`

  auto lhs_reg = parse_argument();
  if (!lhs_reg.has_value())
    return false;

  auto rhs_reg = parse_argument();
  if (!rhs_reg.has_value())
    return false;

  m_program_builder.add_concat(output_reg, lhs_reg.value(), rhs_reg.value());
  return true;
}

/// Grammar:
/// ```
/// slice-expression := "SELECT" INTEGER register
/// ```
bool Parser::parse_select_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_SELECT);
  consume(); // eat `SELECT`

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto i = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  const auto input_reg = parse_argument();
  if (!input_reg.has_value())
    return false;

  m_program_builder.add_select(output_reg, i, input_reg.value());
  return true;
}

/// Grammar:
/// ```
/// slice-expression := "SLICE" INTEGER INTEGER register
/// ```
bool Parser::parse_slice_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_SLICE);
  consume(); // eat `SLICE`

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto first = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto end = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  const auto input_reg = parse_argument();
  if (!input_reg.has_value())
    return false;

  m_program_builder.add_slice(output_reg, first, end, input_reg.value());
  return true;
}

/// Grammar:
/// ```
/// ram-expression := "RAM" INTEGER INTEGER register
/// ```
bool Parser::parse_rom_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_ROM);
  consume(); // eat `ROM`

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto addr_size = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto word_size = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  const auto read_addr_reg = parse_argument();
  if (!read_addr_reg.has_value())
    return false;

  m_program_builder.add_rom(output_reg, addr_size, word_size, read_addr_reg.value());
  return true;
}

/// Grammar:
/// ```
/// ram-expression := "RAM" INTEGER INTEGER register register register register
/// ```
bool Parser::parse_ram_expression(reg_t output_reg) {
  assert(m_token.kind == TokenKind::KEY_RAM);
  consume(); // eat `RAM`

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto addr_size = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  if (m_token.kind != TokenKind::INTEGER) {
    emit_unexpected_token_error(m_token, "an integer constant");
    return false;
  }

  const auto word_size = parse_integer_literal<uint32_t>(m_token.spelling);
  consume(); // eat INTEGER

  const auto read_addr_reg = parse_argument();
  if (!read_addr_reg.has_value())
    return false;

  const auto write_enable_reg = parse_argument();
  if (!write_enable_reg.has_value())
    return false;

  const auto write_addr_reg = parse_argument();
  if (!write_addr_reg.has_value())
    return false;

  const auto write_data_reg = parse_argument();
  if (!write_data_reg.has_value())
    return false;

  m_program_builder.add_ram(output_reg, addr_size, word_size, read_addr_reg.value(), write_enable_reg.value(),
                            write_addr_reg.value(), write_data_reg.value());
  return true;
}

void Parser::consume() {
  m_lexer.tokenize(m_token);
}

void Parser::emit_unexpected_token_error(const Token &token, std::string_view expected_token_name) {
  m_report_manager.report(ReportSeverity::ERROR)
      .with_location(token.position)
      .with_span({token.position, (uint32_t)token.spelling.size()})
      .with_message("unexpected token; expected {}", expected_token_name)
      .finish()
      .exit();
}
