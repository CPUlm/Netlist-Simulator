#ifndef NETLIST_DIAGNOSTIC_CONTEXT_HPP
#define NETLIST_DIAGNOSTIC_CONTEXT_HPP

#include <string_view>

#include "line_map.hpp"
#include "token.hpp"

class DiagnosticContext {
public:
  void set_file_info(std::string_view file_name, std::string_view file_content);

  /// Emits an error at the given source location with the given message.
  /// If the source location is unknown, you can just give INVALID_LOCATION.
  void error_at(SourceLocation location, std::string_view message);

private:
  /// Fills the line map if it is not already.
  void fill_line_map_if_needed();
  /// Gets the text of the requested line (1-numbered) for the current source file.
  [[nodiscard]] std::string_view get_line_at(uint32_t line_number) const;

private:
  std::string_view m_file_name;
  std::string_view m_file_content;
  LineMap m_line_map;
  bool m_line_map_filled = false;
};

#endif // NETLIST_DIAGNOSTIC_CONTEXT_HPP
