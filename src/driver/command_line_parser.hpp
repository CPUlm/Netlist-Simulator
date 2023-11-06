#ifndef NETLIST_SRC_DRIVER_COMMAND_LINE_PARSER_HPP
#define NETLIST_SRC_DRIVER_COMMAND_LINE_PARSER_HPP

#include "report.hpp"

struct CommandLineOptions {
  std::string_view input_file;
  std::string_view backend = "interpreter";
  bool syntax_only = false;
  bool dependency_graph = false;
  bool schedule = false;
  bool timeit = false;
  bool fast = false;
  size_t cycles = 0;
};

class CommandLineParser {
public:
  CommandLineParser(ReportManager &report_manager, int argc, const char *argv[]);

  CommandLineOptions parse();

private:
  int parse_option(std::string_view option, int index);
  [[nodiscard]] std::string_view get_argument(std::string_view option, int index) const;

  void print_help() const;
  void print_version() const;

private:
  ReportManager &m_report_manager;
  const char **m_argv;
  int m_argc;
  CommandLineOptions m_options;
};

#endif // NETLIST_SRC_DRIVER_COMMAND_LINE_PARSER_HPP
