#ifndef NETLIST_SRC_UTILS_HPP
#define NETLIST_SRC_UTILS_HPP

/// Returns true if the given ASCII character is a valid binary digit.
[[nodiscard]] static inline bool is_bin_digit(char ch) {
  return ch == '0' || ch == '1';
}

/// Returns true if the given ASCII character is a valid decimal digit.
[[nodiscard]] static inline bool is_digit(char ch) {
  return (ch >= '0' && ch <= '9');
}

/// Returns true if the given ASCII character is a valid hexadecimal digit.
[[nodiscard]] static inline bool is_hex_digit(char ch) {
  return is_digit(ch) || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F');
}

#endif // NETLIST_SRC_UTILS_HPP
