#include "command_line_parser.hpp"
#include "version.hpp"

#include <cassert>
#include <charconv>

CommandLineParser::CommandLineParser(ReportManager &report_manager, int argc, const char *argv[])
    : m_report_manager(report_manager), m_argc(argc), m_argv(argv) {
  assert(argc > 0 && argv != nullptr);
}

CommandLineOptions CommandLineParser::parse() {
  bool should_parse_options = true;
  for (int i = 1; i < m_argc; ++i) {
    const std::string_view argument = m_argv[i];

    if (argument == "--") {
      should_parse_options = false;
      continue;
    }

    if (should_parse_options && argument.starts_with('-')) {
      // This is a command line option.
      const int eaten_arguments = parse_option(argument, i);
      i += eaten_arguments;
    } else {
      // Not a command line option. In our case, we interpret it as an input file.

      if (!m_options.input_file.empty()) {
        m_report_manager.report(ReportSeverity::ERROR)
            .with_message("only a single Netlist input file is allowed")
            .finish()
            .print();
        print_help();
        exit(EXIT_FAILURE);
      }

      m_options.input_file = argument;
    }
  }

  if (m_options.input_file.empty()) {
    m_report_manager.report(ReportSeverity::ERROR).with_message("missing a Netlist input file").finish().print();
    print_help();
    exit(EXIT_FAILURE);
  }

  return m_options;
}

int CommandLineParser::parse_option(std::string_view option, int index) {
  if (option == "-h" || option == "--help") {
    print_help();
    exit(EXIT_SUCCESS);
  } else if (option == "-v" || option == "--version") {
    print_version();
    exit(EXIT_SUCCESS);
  } else if (option == "-n" || option == "--cycles") {
    const std::string_view argument = get_argument(option, index);

    const auto result = std::from_chars(argument.data(), argument.data() + argument.size(), m_options.cycles);
    if (result.ec != std::errc() || result.ptr != argument.data() + argument.size()) {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_message("invalid argument to `{}', expected an integer", option)
          .finish()
          .exit();
    }

    return 1; // one argument
  } else if (option == "--backend") {
    const std::string_view argument = get_argument(option, index);

    if (argument != "interpreter") {
      m_report_manager.report(ReportSeverity::ERROR)
          .with_message("invalid argument to `{}', expected a valid backend name", option)
          .finish()
          .exit();
    }

    m_options.backend = argument;
    return 1; // one argument
  } else if (option == "--syntax-only") {
    m_options.syntax_only = true;
  } else if (option == "--dep-graph") {
    m_options.dependency_graph = true;
  } else if (option == "--schedule") {
    m_options.schedule = true;
  } else if (option == "--timeit") {
    m_options.timeit = true;
  } else if (option == "--fast") {
    m_options.fast = true;
  } else {
    m_report_manager.report(ReportSeverity::ERROR).with_message("unknown option `{}'", option).finish().print();
    print_help();
    exit(EXIT_FAILURE);
  }

  // By default, an option takes no arguments.
  return 0;
}

std::string_view CommandLineParser::get_argument(std::string_view option, int index) const {
  if (index >= m_argc) {
    m_report_manager.report(ReportSeverity::ERROR)
        .with_message("option `{}' takes one argument, but it is missing", option)
        .finish()
        .print();
    print_help();
    exit(EXIT_FAILURE);
  }

  return m_argv[index + 1];
}

static void print_help_line(std::string_view option, std::string_view description) {
  fmt::println("  {:<25}{}", option, description);
}

void CommandLineParser::print_help() const {
  fmt::println("USAGE: {} [options] input_file", m_argv[0]);
  fmt::println("");
  fmt::println("A simulator for netlists.");
  fmt::println("");
  fmt::println("List of options:");
  print_help_line("-h, --help", "Show this message.");
  print_help_line("-v, --version", "Show the version of the program.");
  print_help_line("-n, --cycles", "The count of cycles to simulate the program.");
  print_help_line("--syntax-only", "Only parses the input file, no scheduling or simulation is done.");
  print_help_line("--dep-graph", "Outputs the dependency graph of the program in Graphviz DOT format.");
  print_help_line("--schedule", "Outputs the scheduled program.");
  print_help_line("--timeit", "Outputs the simulation measured time.");
  print_help_line("--fast", "Enables fast mode when there is no inputs.");
  fmt::println("");
  fmt::println("List of backends:");
  print_help_line("interpreter", "The classical interpreter backend, slow but the more complete.");
}

void CommandLineParser::print_version() const {
  fmt::println("Netlist++, version {} ({})", NETLIST_VERSION, NETLIST_GIT_COMMIT);
  fmt::println("Maintainer: Hubert Gruniaux <hubert@gruniaux.fr>");
  fmt::println("See https://github.com/desfreng/netlist");
}
