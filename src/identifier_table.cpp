#include "identifier_table.hpp"

#include <cstring>

IdentifierInfo::IdentifierInfo(std::string_view spelling) {
  m_spelling_len = spelling.size();
  memcpy(m_spelling, spelling.data(), spelling.size());
  m_spelling[spelling.size()] = '\0';
}

IdentifierInfo &IdentifierTable::get(std::string_view spelling) {
  const auto it = m_mapping.find(spelling);
  if (it != m_mapping.end())
    return *it->second;

  auto *identifier = m_allocator.alloc_with_extra_size<IdentifierInfo>(
      sizeof(char) * spelling.size());
  new (identifier) IdentifierInfo(spelling);

  m_mapping.insert({identifier->get_spelling(), identifier});
  return *identifier;
}

void IdentifierTable::register_keywords() {
#define KEYWORD(spelling)                                                      \
  get(#spelling).set_token_kind(TokenKind::KEY_##spelling);
#include "keywords.def"
}
