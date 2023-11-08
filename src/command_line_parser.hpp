#ifndef NETLIST_SRC_COMMAND_LINE_PARSER_HPP
#define NETLIST_SRC_COMMAND_LINE_PARSER_HPP

#include <vector>

#include "report.hpp"

enum CommandAction {
  Simulate,
  DotExport,
  PrintFile,
  Schedule,
  NoAction
};

class CommandLineParser {
public:
  explicit CommandLineParser(int argc, const char *argv[]);

  [[nodiscard]] CommandAction get_action() const {
    return action;
  }

  [[nodiscard]] std::string get_netlist_file() const {
    return netlist_file;
  }

  [[nodiscard]] std::vector<std::string_view> get_inputs() const {
    return input_files;
  }

  [[nodiscard]] bool cycle_amount_defined() const {
    return cycle.has_value();
  }

  [[nodiscard]] size_t get_cycle_amount() const {
    return cycle.value();
  }

private:
  [[nodiscard]] static std::string build_help(std::string_view prog_name);

  [[nodiscard]] CommandAction parse_flag(std::string_view flag, const std::string &help_str) const;

  void parse_number(int current_argc, int argc, const char *argv[], const std::string &help_str);
  void parse_files(int current_argc, int argc, const char *argv[]);

  const ReportContext ctx;
  std::string netlist_file;
  std::vector<std::string_view> input_files;
  std::optional<size_t> cycle;
  CommandAction action;
};

#endif //NETLIST_SRC_COMMAND_LINE_PARSER_HPP
