#ifndef NETLIST_SRC_DOT_PRINTER_HPP
#define NETLIST_SRC_DOT_PRINTER_HPP

#include <iostream>
#include "program.hpp"

class DotPrinter {
public:
  explicit DotPrinter(const Program::ptr &p) : m_prog(p) {};

  /// Input Variable are in bold. Output Variable are boxed. Arrow mark dependence.
  void print(std::ostream& out = std::cout);
private:
  const Program::ptr &m_prog;
};

#endif //NETLIST_SRC_DOT_PRINTER_HPP
