#include "diagnostic_context.hpp"

#include <iostream>
#include <iomanip>

void DiagnosticContext::set_file_info(std::string_view file_name,
                                      std::string_view file_content) {
  m_file_name = file_name;
  m_file_content = file_content;
}

void DiagnosticContext::error_at(SourceLocation location,
                                 std::string_view message) {
  std::cerr << "\x1b[1;31merror:\x1b[0m " << message << "\n";

  if (location == INVALID_LOCATION) {
    std::cerr << std::endl;
    return;
  }

  fill_line_map_if_needed();
  uint32_t line_number, column_number;
  m_line_map.get_line_and_column_numbers(location, line_number, column_number);
  std::cerr << "  at " << m_file_name << ":" << line_number << ":"
            << column_number << "\n";
  std::cerr << "     |\n";
  std::cerr << std::setw(4) << std::setfill(' ') << line_number << " | " << get_line_at(line_number) << "\n";
  std::cerr << "     |\n";
  std::cerr << std::endl;
}

void DiagnosticContext::fill_line_map_if_needed() {
  if (m_line_map_filled)
    return;

  m_line_map.prefill(m_file_content);
  m_line_map_filled = true;
}

std::string_view DiagnosticContext::get_line_at(uint32_t line_number) const {
  int32_t start_position = m_line_map.get_line_start_position(line_number);

  size_t line_length = 0;
  const char *begin = m_file_content.data() + start_position;
  const char *it = begin;
  while (*it != '\0' && *it != '\n' && *it != '\r') {
    ++line_length;
    ++it;
  }

  return {begin, line_length};
}
