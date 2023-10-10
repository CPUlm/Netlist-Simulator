#ifndef NETLIST_LINE_MAP_HPP
#define NETLIST_LINE_MAP_HPP

#include <cstdint>
#include <string_view>
#include <vector>

/// PLineMap provides functions to convert between character positions and line
/// numbers for a compilation unit.
///
/// Character m_positions are a 0-based byte offset in the source file.
/// Line and column numbers are 1-based like many code editors for convenience.
///
/// The line map is populated by the lexer calling the PLineMap::add_newline()
/// function. This one takes as a parameter the position of the beginning of a
/// new line with the constraint that the new position MUST BE greater than
/// the previous reported position (that way the internal line map m_positions
/// are guaranteed to be sorted).
class LineMap {
public:
  /// Adds a new line position (the position of the first byte of the newline,
  /// that is the position just after the character `\n` or `\r\n`).
  void add_newline(uint32_t start_line_position);

  /// Gets the line and column number corresponding to the given `position` byte
  /// position. Both line and column numbers are 1-based.
  void get_line_and_column_numbers(uint32_t position, uint32_t &line_number,
                                   uint32_t &column_number) const;
  /// Same as get_line_and_column_numbers().
  [[nodiscard]] uint32_t get_line_number(uint32_t position) const;
  /// Same as get_line_and_column_numbers().
  [[nodiscard]] uint32_t get_column_number(uint32_t position) const;
  /// Gets the position of the first byte at the given line (1-based number).
  [[nodiscard]] uint32_t get_line_start_position(uint32_t line_number) const;

  /// Prefills the line map with the line endings found in the given buffer.
  /// The LF, CR and CR-LF line endings are recognized.
  void prefill(std::string_view buffer);

  /// Clears the line map.
  void clear();

private:
  /// Does a binary search on the positions.
  [[nodiscard]] uint32_t search_rightmost(uint32_t position) const;

  std::vector<uint32_t> m_positions;
};

#endif // NETLIST_LINE_MAP_HPP
