#ifndef NETLIST_IDENTIFIER_TABLE_HPP
#define NETLIST_IDENTIFIER_TABLE_HPP

#include <string_view>
#include <unordered_map>

#include "bump_allocator.hpp"
#include "token.hpp"

struct IdentifierInfo {
public:
  /// Returns the UTF-8 encoded spelling of the identifier (NUL-terminated).
  [[nodiscard]] std::string_view get_spelling() const {
    return {m_spelling, m_spelling_len};
  }

  /// Returns the token kind associated to this identifier. Either IDENTIFIER or
  /// a keyword token kind (KEY_*).
  [[nodiscard]] TokenKind get_token_kind() const { return m_token_kind; }
  /// Sets the token kind of this identifier, kind must be either IDENTIFIER or
  /// a keyword token kind (KEY_*).
  void set_token_kind(TokenKind kind) { m_token_kind = kind; }

private:
  friend class IdentifierTable;
  explicit IdentifierInfo(std::string_view spelling);

private:
  TokenKind m_token_kind = TokenKind::IDENTIFIER;
  size_t m_spelling_len;
  char m_spelling[1];
};

/// A string interner for lexed identifiers
class IdentifierTable {
public:
  [[nodiscard]] IdentifierInfo &get(std::string_view spelling);

  /// Registers the keyword of the netlist language.
  void register_keywords();

private:
  BumpAllocator m_allocator;
  std::unordered_map<std::string_view, IdentifierInfo *> m_mapping;
};

#endif // NETLIST_IDENTIFIER_TABLE_HPP
