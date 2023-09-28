#include "line_map.hpp"

#include <cassert>

void LineMap::add_newline(uint32_t start_line_position) {
  m_positions.push_back(start_line_position);
}

void LineMap::get_line_and_column_numbers(uint32_t position,
                                          uint32_t &line_number,
                                          uint32_t &column_number) const {
  // Handle simple cases:
  if (m_positions.empty() || position < m_positions[0]) {
    line_number = 1;
    column_number = position + 1;
    return;
  } else if (position >= m_positions.back()) {
    line_number = static_cast<uint32_t>(m_positions.size() + 1);
    column_number = position - m_positions.back() + 1;
    return;
  }

  // General case (fallback to a binary search):
  const uint32_t upper_bound = search_rightmost(position);
  line_number = upper_bound + 2;
  column_number = position - m_positions[upper_bound] + 1;
}

uint32_t LineMap::get_line_number(uint32_t position) const {
  uint32_t line_number, column_number;
  get_line_and_column_numbers(position, line_number, column_number);
  return line_number;
}

uint32_t LineMap::get_column_number(uint32_t position) const {
  uint32_t line_number, column_number;
  get_line_and_column_numbers(position, line_number, column_number);
  return column_number;
}

uint32_t LineMap::get_line_start_position(uint32_t line_number) const {
  assert(line_number > 0 && line_number <= (m_positions.size() + 1));

  if (line_number == 1)
    return 0;
  else
    return m_positions[line_number - 2];
}

uint32_t LineMap::search_rightmost(uint32_t position) const {
  uint32_t left = 0;
  uint32_t right = static_cast<uint32_t>(m_positions.size());

  while (left < right) {
    const uint32_t middle = (left + right) / 2;
    if (m_positions[middle] > position) {
      right = middle;
    } else {
      left = middle + 1;
    }
  }

  return right - 1;
}

void LineMap::prefill(std::string_view buffer) {
  for (uint32_t i = 0; i < buffer.size(); ++i) {
    switch (buffer[i]) {
    case '\n': // LF line ending
      add_newline(i + 1);
      break;
    case '\r':
      ++i;
      if (buffer[i] == '\n') { // CR-LF line ending
        add_newline(i + 1);
      } else { // CR line ending
        add_newline(i);
      }
      break;
    }
  }
}
