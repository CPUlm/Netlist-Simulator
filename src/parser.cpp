#include "parser.hpp"

const std::unordered_map<TokenKind, std::string> token_spelling = {
    {TokenKind::EOI, "End-Of-File"},
    {TokenKind::IDENTIFIER, "Identifier (such as '_l_10')"},
    {TokenKind::INTEGER, "Integer (such as '42')"},
    {TokenKind::BINARY_CONSTANT, "Binary Constant (such as '011011' or '1')"},
    {TokenKind::DECIMAL_CONSTANT, "Decimal Constant (such as '0d215')"},
    {TokenKind::HEXADECIMAL_CONSTANT, "Hexadecimal Constant (such as '0xf2f')"},

    {TokenKind::EQUAL, "="},
    {TokenKind::COMMA, ","},
    {TokenKind::COLON, ":"},

    {TokenKind::KEY_INPUT, "INPUT"},
    {TokenKind::KEY_OUTPUT, "OUTPUT"},
    {TokenKind::KEY_VAR, "VAR"},
    {TokenKind::KEY_IN, "IN"},

    {TokenKind::KEY_NOT, "NOT"},
    {TokenKind::KEY_AND, "AND"},
    {TokenKind::KEY_NAND, "NAND"},
    {TokenKind::KEY_OR, "OR"},
    {TokenKind::KEY_XOR, "XOR"},

    {TokenKind::KEY_MUX, "MUX"},
    {TokenKind::KEY_REG, "REG"},
    {TokenKind::KEY_CONCAT, "CONCAT"},
    {TokenKind::KEY_SELECT, "SELECT"},
    {TokenKind::KEY_SLICE, "SLICE"},
    {TokenKind::KEY_ROM, "ROM"},
    {TokenKind::KEY_RAM, "RAM"},
};

Parser::Parser(ReportContext &context, Lexer &lexer)
    : m_context(context), m_lexer(lexer) {
  // Gets the first token
  m_lexer.tokenize(m_token);
}

void Parser::consume() { m_lexer.tokenize(m_token); }

void Parser::assert(const std::set<TokenKind> &token_set) const noexcept {
  if (!token_set.contains(m_token.kind)) {
    std::string tokens = token_spelling.at(*token_set.begin());

    if (token_set.size() > 1) {
      for (auto it = ++token_set.begin(); it != token_set.end(); it++) {
        tokens.append("' or '" + token_spelling.at(*it));
      }
    }

    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Unexpected token. Found : '{}', expected : '{}'",
                      m_token.spelling, tokens)
        .build()
        .exit();
  }
}

void Parser::assert(TokenKind token) const noexcept {
  if (m_token.kind != token) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Unexpected token. Found : '{}', expected : '{}'",
                      m_token.spelling, token_spelling.at(token))
        .build()
        .exit();
  }
}

Program::ptr Parser::parse_program() {
  vars.clear(); // Remove any existing variable
  Program::ptr p(new Program());

  // m_token.kind = TokenKind::KEY_INPUT
  const var_ref_map in_refs = parse_inputs_references();
  // m_token.kind = TokenKind::KEY_OUTPUT
  const var_ref_map out_refs = parse_outputs_references();
  // m_token.kind = TokenKind::KEY_VAR
  parse_variable_declaration();
  // m_token.kind = TokenKind::KEY_IN
  build_intput_output_list(p, in_refs, out_refs);

  // Now we parse each equation
  parse_equations(p);
  // m_token.kind = TokenKind::EOI

  // Check that all variables have either an equation or are an input
  return p;
}

Parser::var_ref_map Parser::parse_inputs_references() noexcept {
  assert(TokenKind::KEY_INPUT);
  consume(); // eat 'INPUT'

  var_ref_map var;

  while (m_token.kind != TokenKind::KEY_OUTPUT) {
    assert(TokenKind::IDENTIFIER);

    if (var.contains(m_token.spelling)) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message(
              "The variable '{}' has already been marked as input at {}.",
              m_token.spelling,
              m_context.get_location(var[m_token.spelling].position))
          .build()
          .exit();
    }

    var[m_token.spelling] = {m_token.spelling, m_token.position};
    consume(); // eat 'IDENTIFIER'

    assert({TokenKind::COMMA, TokenKind::KEY_OUTPUT});

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat 'COMMA'
    }
  }

  return var;
}

Parser::var_ref_map Parser::parse_outputs_references() noexcept {
  assert(TokenKind::KEY_OUTPUT);
  consume(); // eat 'OUTPUT'

  var_ref_map var;

  while (m_token.kind != TokenKind::KEY_VAR) {
    assert(TokenKind::IDENTIFIER);
    if (var.contains(m_token.spelling)) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message(
              "The variable '{}' has already been marked as output at {}.",
              m_token.spelling,
              m_context.get_location(var[m_token.spelling].position))
          .build()
          .exit();
    }

    var[m_token.spelling] = {m_token.spelling, m_token.position};
    consume(); // eat 'IDENTIFIER'

    assert({TokenKind::COMMA, TokenKind::KEY_VAR});

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat 'COMMA'
    }
  }

  return var;
}

void Parser::parse_variable_declaration() noexcept {
  assert(TokenKind::KEY_VAR);
  consume(); // eat 'VAR'

  std::unordered_map<std::string_view, Parser::VariableDeclaration> var_decl;

  while (m_token.kind != TokenKind::KEY_IN) {
    assert(TokenKind::IDENTIFIER);

    VariableDeclaration v = {};
    v.spelling = m_token.spelling;
    v.position = m_token.position;
    consume(); // eat 'IDENTIFIER'

    if (m_token.kind == TokenKind::COLON) {
      consume(); // eat ':'
      v.size = parse_bus_size();
    } else {
      v.size = 1;
    }

    if (var_decl.contains(v.spelling)) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(v.position)
          .with_message("The variable '{}' has already been declared at {}.",
                        v.spelling,
                        m_context.get_location(var_decl[v.spelling].position))
          .build()
          .exit();
    }

    vars.try_emplace(v.spelling, std::make_shared<Variable>(v.size, v.spelling));
    var_decl[v.spelling] = v;

    assert({TokenKind::COMMA, TokenKind::KEY_IN});

    if (m_token.kind == TokenKind::COMMA) {
      consume(); // eat 'COMMA'
      assert(TokenKind::IDENTIFIER);
    }
  }
}

Constant::ptr Parser::parse_bin_const(bus_size_t size) noexcept {
  assert(TokenKind::BINARY_CONSTANT);
  auto value = parse_int<value_t>(2); // We parse in base 2 here

  if (value > Bus::max_value(size)) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("The binary value '{}' (decimal : {}) is too large to fit in a bus size of {}."
                      " The maximum authorised value for the variable is : {}",
                      m_token.spelling, value, size, Bus::max_value(size))
        .build()
        .exit();
  }

  consume();
  return std::make_shared<Constant>(size, value);
}

Constant::ptr Parser::parse_dec_const(bus_size_t size) noexcept {
  assert(TokenKind::DECIMAL_CONSTANT);
  auto value = parse_int<value_t>(10); // We parse in base 10 here

  if (value > Bus::max_value(size)) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("The decimal value '{}' is too large to fit in a bus size of {}."
                      " The maximum authorised value for the variable is : {}",
                      m_token.spelling, value, size, Bus::max_value(size))
        .build()
        .exit();
  }

  consume();
  return std::make_shared<Constant>(size, value);
}

Constant::ptr Parser::parse_hex_const(bus_size_t size) noexcept {
  assert(TokenKind::HEXADECIMAL_CONSTANT);
  auto value = parse_int<value_t>(16); // We parse in base 16 here

  if (value > Bus::max_value(size)) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("The hexadecimal value '{}' (decimal : {}) is too large to fit in a bus size of {}."
                      " The maximum authorised value for the variable is : {}",
                      m_token.spelling, value, size, Bus::max_value(size))
        .build()
        .exit();
  }

  consume();
  return std::make_shared<Constant>(size, value);
}

Variable::ptr Parser::parse_var() noexcept {
  assert(TokenKind::IDENTIFIER);

  if (!vars.contains(m_token.spelling)) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Undefined variable {}.", m_token.spelling)
        .build()
        .exit();
  }

  consume();
  return vars.at(m_token.spelling);
}

Variable::ptr Parser::parse_var(bus_size_t size) noexcept {
  const SourcePosition pos = m_token.position;
  auto v = parse_var();
  assert_var_size(v, size, pos);
  return v;
}

void Parser::assert_var_size(const Variable::ptr &var, bus_size_t size, const SourcePosition &pos) noexcept {
  if (var->get_bus_size() != size) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(pos)
        .with_message("Variable '{}' (declared bus size : {}) should have a bus size of {}.",
                      var->get_name(), var->get_bus_size(), size)
        .build()
        .exit();
  }
}

void Parser::assert_var_size_greater_than(const Variable::ptr &var,
                                          bus_size_t minimal_size,
                                          const SourcePosition &pos) noexcept {
  if (var->get_bus_size() < minimal_size) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(pos)
        .with_message("Variable '{}' (declared bus size : {}) should have a bus size >= {} in this context.",
                      var->get_name(), var->get_bus_size(), minimal_size)
        .build()
        .exit();
  }
}

Argument::ptr Parser::parse_argument(bus_size_t expected_size) noexcept {
  assert({TokenKind::IDENTIFIER, TokenKind::BINARY_CONSTANT, TokenKind::DECIMAL_CONSTANT,
          TokenKind::HEXADECIMAL_CONSTANT, TokenKind::INTEGER});

  if (m_token.kind == TokenKind::IDENTIFIER) {
    return parse_var(expected_size);
  } else if (m_token.kind == TokenKind::BINARY_CONSTANT) {
    return parse_bin_const(expected_size);
  } else if (m_token.kind == TokenKind::DECIMAL_CONSTANT) {
    return parse_dec_const(expected_size);
  } else if (m_token.kind == TokenKind::HEXADECIMAL_CONSTANT) {
    return parse_hex_const(expected_size);
  } else {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Missing decimal prefix for the constant '{}'", m_token.spelling)
        .build()
        .exit();
  }
}

bus_size_t Parser::parse_bus_size() noexcept {
  assert({TokenKind::INTEGER, TokenKind::BINARY_CONSTANT}); // '10' can will be interpreted as a BinaryConstant here

  auto bs = parse_int<bus_size_t>(10); // We parse in base 10 here.

  if (bs > max_bus_size) {
    m_context.report(ReportSeverity::ERROR)
        .with_location(m_token.position)
        .with_message("Integer '{}' is too big to be a bus size. Max bus size authorised : '{}'", bs, max_bus_size)
        .build()
        .exit();
  }

  consume();
  return bs;
}

void Parser::build_intput_output_list(Program::ptr &p, const var_ref_map &in_refs, const var_ref_map &out_refs) {
  for (auto &in_pair : in_refs) {
    // We only iterate over the VariableReference value of the hashtable.
    const VariableReference &i_ref = in_pair.second;
    if (!vars.contains(i_ref.spelling)) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(i_ref.position)
          .with_message(
              "Missing declaration of input '{}' in the variable section.",
              i_ref.spelling)
          .build()
          .exit();
    }

    p->m_input.emplace_back(vars.at(i_ref.spelling));
  }

  for (auto &out_pair : out_refs) {
    // We only iterate over the VariableReference value of the hashtable.
    const VariableReference &o_ref = out_pair.second;
    if (!vars.contains(o_ref.spelling)) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(o_ref.position)
          .with_message(
              "Missing declaration of output '{}' in the variable section.",
              o_ref.spelling)
          .build()
          .exit();
    }

    p->m_output.emplace_back(vars.at(o_ref.spelling));
  }
}

void Parser::parse_equations(Program::ptr &p) {
  assert(TokenKind::KEY_IN);
  consume(); // eat 'IN'

  while (m_token.kind != TokenKind::EOI) {
    assert(TokenKind::IDENTIFIER);
    if (!vars.contains(m_token.spelling)) {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message("Assigment of undefined variable '{}'",
                        m_token.spelling)
          .build()
          .exit();
    }
    const Variable::ptr &assigment_var = vars.at(m_token.spelling);
    consume(); // eat the variable
    assert(TokenKind::EQUAL);
    consume(); // eat '='

    switch (m_token.kind) {
    case TokenKind::IDENTIFIER: {
      const Variable::ptr r_var = parse_var(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<ArgExpression>(r_var));
      break;
    }

    case TokenKind::BINARY_CONSTANT: {
      const Constant::ptr cst = parse_bin_const(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<ArgExpression>(cst));
      break;
    }

    case TokenKind::DECIMAL_CONSTANT: {
      const Constant::ptr cst = parse_dec_const(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<ArgExpression>(cst));
      break;
    }

    case TokenKind::HEXADECIMAL_CONSTANT: {
      const Constant::ptr cst = parse_hex_const(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<ArgExpression>(cst));
      break;
    }

    case TokenKind::INTEGER: {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message("Missing decimal prefix during affectation of the variable {} with integer constant '{}'",
                        assigment_var->get_name(), m_token.spelling)
          .build()
          .exit();
    }

    case TokenKind::KEY_NOT: {
      const Argument::ptr arg = parse_argument(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<NotExpression>(arg));
      break;
    }

    case TokenKind::KEY_AND: {
      const Argument::ptr lhs = parse_argument(assigment_var->get_bus_size());
      const Argument::ptr rhs = parse_argument(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<BinOpExpression>(BinOpExpression::BinOpKind::AND, lhs, rhs));
      break;
    }

    case TokenKind::KEY_NAND: {
      const Argument::ptr lhs = parse_argument(assigment_var->get_bus_size());
      const Argument::ptr rhs = parse_argument(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<BinOpExpression>(BinOpExpression::BinOpKind::NAND, lhs, rhs));
      break;
    }

    case TokenKind::KEY_OR: {
      const Argument::ptr lhs = parse_argument(assigment_var->get_bus_size());
      const Argument::ptr rhs = parse_argument(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<BinOpExpression>(BinOpExpression::BinOpKind::OR, lhs, rhs));
      break;
    }

    case TokenKind::KEY_XOR: {
      const Argument::ptr lhs = parse_argument(assigment_var->get_bus_size());
      const Argument::ptr rhs = parse_argument(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<BinOpExpression>(BinOpExpression::BinOpKind::XOR, lhs, rhs));
      break;
    }

    case TokenKind::KEY_MUX: {
      const Argument::ptr choice = parse_argument(1);
      const Argument::ptr true_branch = parse_argument(assigment_var->get_bus_size());
      const Argument::ptr false_branch = parse_argument(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<MuxExpression>(choice, true_branch, false_branch));
      break;
    }

    case TokenKind::KEY_REG: {
      const Variable::ptr var = parse_var(assigment_var->get_bus_size());
      p->m_eq.try_emplace(assigment_var, std::make_unique<RegExpression>(var));
      break;
    }

    case TokenKind::KEY_CONCAT: {
      const SourcePosition pos = m_token.position;
      const Variable::ptr var_beg = parse_var();

      if (var_beg->get_bus_size() >= assigment_var->get_bus_size()) {
        m_context.report(ReportSeverity::ERROR)
            .with_location(pos)
            .with_message("Variable '{}' (declared bus size : {}) should have a bus size lower than {} to make sense"
                          " in this CONCAT expression.",
                          var_beg->get_name(), var_beg->get_bus_size(), assigment_var->get_bus_size())
            .build()
            .exit();
      }

      const Variable::ptr var_end = parse_var(assigment_var->get_bus_size() - var_beg->get_bus_size());

      p->m_eq.try_emplace(assigment_var, std::make_unique<ConcatExpression>(var_beg, var_end));
      break;
    }

    case TokenKind::KEY_SELECT: {
      bus_size_t index = parse_bus_size();

      if (m_token.kind == TokenKind::IDENTIFIER) { // It's a variable
        const SourcePosition pos = m_token.position;
        const Variable::ptr var = parse_var();

        assert_var_size_greater_than(var, index + 1, pos);

        p->m_eq.try_emplace(assigment_var, std::make_unique<SelectExpression>(index, var));
      } else { // We expect a constant
        // Cannot be a variable, so it's okay to enforce this bus size
        const Argument::ptr cst = parse_argument(max_bus_size);
        p->m_eq.try_emplace(assigment_var, std::make_unique<SelectExpression>(index, cst));
      }

      break;
    }

    case TokenKind::KEY_SLICE: {
      bus_size_t beg = parse_bus_size();
      bus_size_t end = parse_bus_size();

      if (m_token.kind == TokenKind::IDENTIFIER) { // It's a variable
        const SourcePosition pos = m_token.position;
        const Variable::ptr var = parse_var();

        assert_var_size_greater_than(var, end + 1, pos);

        p->m_eq.try_emplace(assigment_var, std::make_unique<SliceExpression>(beg, end, var));
      } else { // We expect a constant
        // Cannot be a variable, so it's okay to enforce this bus size
        const Argument::ptr cst = parse_argument(max_bus_size);
        p->m_eq.try_emplace(assigment_var, std::make_unique<SliceExpression>(beg, end, cst));
      }

      break;
    }

    case TokenKind::KEY_ROM: {
      const bus_size_t addr_size = parse_bus_size();

      const SourcePosition pos = m_token.position;
      const bus_size_t word_size = parse_bus_size();

      if (word_size != assigment_var->get_bus_size()) {
        m_context.report(ReportSeverity::ERROR)
            .with_location(pos)
            .with_message("Bus size mismatch between the word_size of this ROM expression (word_size : {}) and the"
                          " target variable '{}' (bus size : {})",
                          word_size, assigment_var->get_name(), assigment_var->get_bus_size())
            .build()
            .exit();
      }

      const Argument::ptr read_addr = parse_argument(addr_size);

      p->m_eq.try_emplace(assigment_var, std::make_unique<RomExpression>(addr_size, word_size, read_addr));
      break;
    }

    case TokenKind::KEY_RAM: {
      const bus_size_t addr_size = parse_bus_size();

      SourcePosition pos = m_token.position;
      const bus_size_t word_size = parse_bus_size();

      if (word_size != assigment_var->get_bus_size()) {
        m_context.report(ReportSeverity::ERROR)
            .with_location(pos)
            .with_message("Bus size mismatch between the word_size of this ROM expression (word_size : {}) and the"
                          " target variable '{}' (bus size : {})",
                          word_size, assigment_var->get_name(), assigment_var->get_bus_size())
            .build()
            .exit();
      }

      const Argument::ptr read_addr = parse_argument(addr_size);
      const Argument::ptr write_enable = parse_argument(1);
      const Argument::ptr write_addr = parse_argument(addr_size);
      const Argument::ptr data = parse_argument(word_size);

      p->m_eq.try_emplace(assigment_var,
                          std::make_unique<RamExpression>(addr_size,
                                                          word_size,
                                                          read_addr,
                                                          write_enable,
                                                          write_addr,
                                                          data));
      break;
    }

    default: {
      m_context.report(ReportSeverity::ERROR)
          .with_location(m_token.position)
          .with_message("Cannot interpret the assigment of variable '{}'.", assigment_var->get_name())
          .build()
          .exit();
    }
    }
  }
}
