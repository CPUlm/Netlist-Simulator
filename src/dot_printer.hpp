#ifndef NETLIST_SRC_DOT_PRINTER_HPP
#define NETLIST_SRC_DOT_PRINTER_HPP

#include <iostream>
#include "program.hpp"

class DotPrinter {
public:
  explicit DotPrinter(const Program::ptr &p, std::ostream &out = std::cout) : m_prog(p), out(out) {};

  /// Input Variable are in bold. Output Variable are boxed. Arrow mark dependence.
  void print();
private:
  const Program::ptr &m_prog;
  std::ostream &out;
};

#endif //NETLIST_SRC_DOT_PRINTER_HPP
