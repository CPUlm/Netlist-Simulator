#ifndef NETLIST_SRC_UTILS_HPP
#define NETLIST_SRC_UTILS_HPP

#include <string>
#include <fstream>

#include "report.hpp"
#include "program.hpp"
#include "simulator.hpp"

// Reads the full content of a file at the given path.
std::string read_file(const ReportContext &ctx, std::string_view path) {
  constexpr std::size_t BUFFER_SIZE = 4096;
  static char buffer[BUFFER_SIZE] = {0};

  std::ifstream f = std::ifstream(path.data());
  f.exceptions(std::ios_base::badbit); // To throw error during read

  if (!f.is_open()) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Error opening file {}", path)
        .with_code(60)
        .build()
        .exit();
  }

  std::string content;
  try {
    while (f.read(buffer, BUFFER_SIZE)) {
      content.append(buffer, 0, f.gcount());
    }
  } catch (std::exception &e) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Error occurred when reading file {} : {}", path, e.what())
        .with_code(61)
        .build()
        .exit();
  }
  content.append(buffer, 0, f.gcount());

  return content;
}

void print_output_values(const Simulator &sim, const Program::ptr &p, std::ostream &out) {
  for (const Variable::ptr &out_var : p->get_outputs()) {
    out << "=> " << out_var->get_name() << " = " << sim.read_value(out_var) << "\n";
  }
}

#endif //NETLIST_SRC_UTILS_HPP
