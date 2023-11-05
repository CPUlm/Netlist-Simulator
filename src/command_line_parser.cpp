#include "command_line_parser.hpp"

#include <charconv>
#include <sstream>

void assert_args_gt(int argc, int i, const ReportContext &ctx, const std::string &help_str) {
  if (argc <= i) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Not enough arguments passed. Expected at least {} arguments.", i)
        .with_code(71)
        .with_help(help_str)
        .build()
        .exit();
  }
}

CommandLineParser::CommandLineParser(int argc, const char *argv[]) : ctx(true), input_files() {
  if (argc <= 0) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("No Argument Passed.")
        .with_code(70)
        .build()
        .exit();
  }

  // argc > 0 (need program name)
  const std::string help_str = build_help(argv[0]);

  assert_args_gt(argc, 1, ctx, help_str); // 1 argument needed to get action so argc >= 2 needed.
  const std::string_view flag = argv[1];
  action = parse_flag(flag, help_str);

  switch (action) {
  case Simulate: {
    assert_args_gt(argc, 2, ctx, help_str); // We need to read argv[2] (so argc > 2)
    std::string_view arg = argv[2];

    if (arg == "-v" || arg == "--verbose") {
      parse_verbose(argc, argv, help_str);
    } else if (arg == "-n" || arg == "--number") {
      parse_number(2, argc, argv, help_str);
    } else {
      parse_files(2, argc, argv);
    }
    break;
  }

  case DotExport:
  case PrintFile:
  case Schedule: {
    if (argc != 3) { // 2 arguments : action & filename
      ctx.report(ReportSeverity::ERROR)
          .with_message("Expected 2 arguments here : action and netlist file.")
          .with_code(72)
          .with_help(help_str)
          .build()
          .exit();
    }

    netlist_file = argv[2];
    break;
  }
  case NoAction:
    break;
  }
}

[[nodiscard]] CommandAction CommandLineParser::parse_flag(std::string_view flag, const std::string &help_str) const {
  if (flag == "-h" || flag == "--help") {
    std::cout << "Usages:" << help_str;
    return NoAction;
  } else if (flag == "-V" || flag == "--version") {
    std::cout << "NetList Simulator, version 1.0\n"
              << "Maintainer : Gabriel Desfrene <gabriel@desfrene.fr>\n";
    return NoAction;
  } else if (flag == "-s" || flag == "--simulate") {
    return Simulate;
  } else if (flag == "-p" || flag == "--print") {
    return PrintFile;
  } else if (flag == "-d" || flag == "--dot") {
    return DotExport;
  } else if (flag == "-c" || flag == "--schedule") {
    return Schedule;
  } else {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Unknown action : '{}'", flag)
        .with_code(73)
        .with_help(help_str)
        .build()
        .exit();
  }
}

[[nodiscard]] std::string CommandLineParser::build_help(std::string_view prog_name) {
  std::stringstream str;
  str
      << "\n"
      << "- To print this help:\n"
      << "    " << prog_name << " [ -h | --help ]\n"
      << "\n"
      << "- To print program version:\n"
      << "    " << prog_name << " [ -V | --version ]\n"
      << "\n"
      << "- To simulate k cycles of a netlist, with data for ROM (and RAM) blocks:\n"
      << "    " << prog_name << " [ -s | --simulate ] [ -v | --verbose ] [ -n k | --number k ] netlist [inputs...]\n"
      << "\n"
      << "- To print a netlist as a graphviz graph (compatible with dot) to stdout:\n"
      << "    " << prog_name << " [ -d | --dot ] netlist\n"
      << "\n"
      << "- To print a netlist file to stdout:\n"
      << "    " << prog_name << " [ -p | --print ] netlist\n"
      << "\n"
      << "- To print the result of the scheduling of the netlist to stdout:\n"
      << "    " << prog_name << " [ -c | --schedule ] netlist\n"
      << "\n"
      << "Please refer to the project notes for the format of all inputs.\n"
      << "If the number of cycle to simulate is undefined, the simulation will loop until a\n"
      << "SIGINT or a SIGTERM is caught.\n"
      << "If the verbose flag is set, all output will be printed at each cycle.\n";

  return str.str();
}

void CommandLineParser::parse_verbose(int argc, const char *argv[], const std::string &help_str) {
  if (argc < 2) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Illegal parse_verbose call.")
        .with_code(93)
        .build()
        .exit();
  }

  std::string_view verb_arg = argv[2];
  if (verb_arg != "-v" && verb_arg != "--verbose") {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Illegal parse_verbose call.")
        .with_code(93)
        .build()
        .exit();
  }

  verbose = true;

  assert_args_gt(argc, 3, ctx, help_str);
  std::string_view arg = argv[3];
  if (arg == "-n" || arg == "--number") {
    parse_number(3, argc, argv, help_str);
  } else {
    parse_files(3, argc, argv);
  }
}

void CommandLineParser::parse_number(int current_argc, int argc, const char *argv[], const std::string &help_str) {
  if (argc < current_argc) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Illegal parse_number call.")
        .with_code(94)
        .build()
        .exit();
  }

  std::string_view verb_arg = argv[current_argc];
  if (verb_arg != "-n" && verb_arg != "--number") {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Illegal parse_number call.")
        .with_code(94)
        .build()
        .exit();
  }

  // Need one more for the number and one more for the netlist file
  assert_args_gt(argc, current_argc + 2, ctx, help_str);

  const char *num_str = argv[current_argc + 1];
  std::size_t arg_size = std::strlen(num_str);
  size_t val;
  auto [ptr, ec] = std::from_chars(num_str, num_str + arg_size, val, 10);
  if (ec != std::errc() || ptr != num_str + arg_size) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Cannot interpret '{}' as a base 10 number.", num_str)
        .with_code(74)
        .with_help(help_str)
        .build()
        .exit();
  } else {
    cycle = val;
  }

  parse_files(current_argc + 2, argc, argv);
}

void CommandLineParser::parse_files(int current_argc, int argc, const char *argv[]) {
  if (argc < current_argc) {
    ctx.report(ReportSeverity::ERROR)
        .with_message("Illegal parse_files call.")
        .with_code(95)
        .build()
        .exit();
  }

  netlist_file = argv[current_argc];

  for (int i = current_argc + 1; i < argc; ++i) {
    input_files.emplace_back(argv[i]);
  }
}
