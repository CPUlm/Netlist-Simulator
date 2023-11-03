#include "dependency_graph.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "simulator/simulator.hpp"

#include <charconv>
#include <fstream>
#include <iostream>

struct CmdOptions {
  std::string_view input_file;
  std::string_view backend = "interpreter";
  bool syntax_only = false;
  bool dependency_graph = false;
  bool schedule = false;
  size_t cycles = 1;
};

static void print_help(const char *argv0) {
  fmt::println("USAGE: {} [options...] input.net", argv0);
  fmt::println("");
  fmt::println("List of options:");
  fmt::println("  -h, --help             Show this message.");
  fmt::println("  -v, --version          Show the version of the program.");
  fmt::println("  --cycles <n>           The count of cycles to simulate the program. The default is 1.");
  fmt::println("  --backend <backend>    Selects a specific simulator backend.");
  fmt::println("  --syntax-only          Only parses the input file, no scheduling or simulation is done.");
  fmt::println("  --dep-graph            Outputs the dependency graph of the program in Graphviz DOT format.");
  fmt::println("  --schedule             Outputs the scheduled program.");
  fmt::println("");
  fmt::println("List of backends:");
  fmt::println("  interpreter            The classical interpreter backend, slow but the more complete.");
}

static void print_version() {
  fmt::println("Netlist++ v1.0");
}

/// The command line parser.
static CmdOptions parse_cmd_args(ReportManager &reports, int argc, const char *argv[]) {
  // Ideally, it will be better to use an external library to do it.
  // Like boost::program_options. But adding another dependency will
  // complicate the build process. So we implement it ourselves, for the
  // better and the worse.

  CmdOptions options;

  // The first element of argv is always, by the system conventions,
  // the path of the program. So, we ignore it when parsing options.
  bool parsing_options = true;
  for (int i = 1; i < argc; ++i) {
    const std::string_view arg = argv[i];

    // All arguments after "--" should not be parsed as options.
    if (parsing_options && arg == "--") {
      parsing_options = false;
      continue;
    }

    if (parsing_options) {
      if (arg == "-h" || arg == "--help") {
        print_help(argv[0]);
        std::exit(EXIT_SUCCESS);
      } else if (arg == "-v" || arg == "--version") {
        print_version();
        std::exit(EXIT_SUCCESS);
      } else if (arg == "--cycles") {
        if (i + 1 >= argc) {
          reports.report(ReportSeverity::ERROR)
              .with_message("command line option `--cycles' requires a parameter")
              .finish()
              .print();
          print_help(argv[0]);
          std::exit(EXIT_FAILURE);
        }

        options.cycles = std::stoull(argv[i++]);
        continue;
      } else if (arg == "--backend") {
        if (i + 1 >= argc) {
          reports.report(ReportSeverity::ERROR)
              .with_message("command line option `--backend' requires a parameter")
              .finish()
              .print();
          print_help(argv[0]);
          std::exit(EXIT_FAILURE);
        }

        options.backend = argv[i++];
        continue;
      } else if (arg == "--syntax-only") {
        options.syntax_only = true;
        continue;
      } else if (arg == "--schedule") {
        options.schedule = true;
        continue;
      } else if (arg == "--dep-graph") {
        options.dependency_graph = true;
        continue;
      }
    }

    if (!options.input_file.empty()) {
      reports.report(ReportSeverity::ERROR)
          .with_message("only a single Netlist input file is allowed")
          .finish()
          .print();
      print_help(argv[0]);
      std::exit(EXIT_FAILURE);
    }

    options.input_file = arg;
  }

  if (options.input_file.empty()) {
    reports.report(ReportSeverity::ERROR).with_message("missing a Netlist input file").finish().print();
    print_help(argv[0]);
    std::exit(EXIT_FAILURE);
  }

  if (options.schedule && options.dependency_graph) {
    reports.report(ReportSeverity::WARNING)
        .with_message("command line options `--schedule' and `--dep-graph' are exclusives")
        .finish()
        .print();
  }

  return options;
}

[[nodiscard]] std::string read_file(std::string_view path) {
  // From https://stackoverflow.com/a/116220/8959979

  constexpr auto read_size = std::size_t(4096);
  auto stream = std::ifstream(path.data());
  stream.exceptions(std::ios_base::badbit);

  if (not stream) {
    throw std::ios_base::failure("file does not exist");
  }

  auto out = std::string();
  auto buf = std::string(read_size, '\0');
  while (stream.read(&buf[0], read_size)) {
    out.append(buf, 0, stream.gcount());
  }
  out.append(buf, 0, stream.gcount());
  return out;
}

int main(int argc, const char *argv[]) {
  std::ostream::sync_with_stdio();

  ReportManager report_manager;
  const auto options = parse_cmd_args(report_manager, argc, argv);

  std::string source_code;
  try {
    source_code = read_file(options.input_file);
  } catch (const std::exception &) {
    report_manager.report(ReportSeverity::ERROR)
        .with_message("failed to read file `{}'", options.input_file)
        .finish()
        .exit();
  }

  report_manager.register_file_info(options.input_file, source_code);
  Lexer lexer(report_manager, source_code.data());
  Parser parser(report_manager, lexer);
  std::shared_ptr<Program> program = parser.parse_program();

  if (!options.syntax_only && program != nullptr) {
    DependencyGraph graph = DependencyGraph::build(program);
    if (options.dependency_graph) {
      graph.dump_dot();
    } else if (options.schedule) {
      graph.schedule(report_manager);
      Disassembler::disassemble(program);
    } else {
      graph.schedule(report_manager);
      Simulator simulator(program);
      fmt::println("Simulating {} cycle(s) of `{}'.", options.cycles, options.input_file);
      fmt::println("Inputs:");
      for (const auto input_reg : program->get_inputs()) {
        bool has_error;
        do {
          fmt::print("  - {} = ", program->get_reg_name(input_reg));
          std::string value_string;
          std::cin >> value_string;

          reg_value_t value;
          const auto result = std::from_chars(value_string.data(), value_string.data() + value_string.size(), value, 2);
          if (result.ec != std::errc() || result.ptr != (value_string.data() + value_string.size())) {
            report_manager.report(ReportSeverity::ERROR)
                .with_message("expected a constant, `{}' is not one", value_string)
                .finish()
                .print();
            has_error = true;
          } else {
            has_error = false;
            simulator.set_register(input_reg, value);
          }
        } while (has_error);
      }

      simulator.execute(options.cycles);

      fmt::println("Outputs:");
      for (const auto output_reg : program->get_outputs()) {
        fmt::println("  - {} = {:b}", program->get_reg_name(output_reg), simulator.get_register(output_reg));
      }
    }
  }

  return EXIT_SUCCESS;
}
