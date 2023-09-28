#include "lexer.hpp"
#include "parser.hpp"
#include "program_printer.hpp"

#include <fstream>
#include <iostream>
#include <optional>

static std::ostream &error(std::ostream &stream) {
  stream << "\x1b[1;31mERROR:\x1b[0m ";
  return stream;
}

/// The different options that can be given to the netlist compiler/simulator.
struct CommandLineOptions {
  std::vector<std::string_view> input_files;
};

/// A simple command line option parser for use by the netlist
/// compiler/simulator. It fills the data structure CommandLineOptions
/// using the command line arguments given to main() function.
///
/// Usage:
/// ```
/// CommandLineOptions cmd_options;
/// CommandLineParser cmd_parser(cmd_options);
/// if (!cmd_parser.parse(argc, argv))
///   return EXIT_FAILURE;
///
/// // Then:
/// options.input_files;
/// // etc.
/// ```
class CommandLineParser {
public:
  explicit CommandLineParser(CommandLineOptions &options)
      : m_options(options) {}

  bool parse(int argc, const char *argv[]) {
    assert(argc > 0);

    m_program_name = argv[0];

    // The first argument is generally, by convention, the program name.
    bool has_error = false;
    for (int i = 1; i < argc; ++i) {
      std::string_view argument = argv[i];
      if (argument.starts_with("-")) {
        has_error |= !parse_option(argument);
        if (has_error)
          break;
      } else {
        m_options.input_files.push_back(argument);
      }
    }

    if (!has_error && post_validation()) {
      return true;
    }

    show_help_message();
    return false;
  }

private:
  bool parse_option(std::string_view option) {
    assert(option.starts_with("-"));

    if (option == "-h" || option == "--help") {
      show_help_message(stdout);
      std::exit(EXIT_SUCCESS);
    } else if (option == "--version") {
      show_version_message();
      std::exit(EXIT_SUCCESS);
    } else {
      std::cerr << error << "unknown option `" << option << "'" << std::endl;
      return false;
    }

    return true;
  }

  bool post_validation() {
    if (m_options.input_files.empty()) {
      std::cerr << error << "no input files" << std::endl;
      return false;
    }

    return true;
  }

  void show_help_message(FILE *file = stderr) {
    fprintf(file, "USAGE %s [options...] [files...]\n", m_program_name.data());
  }

  void show_version_message() {
    printf("Netlist compiler/simulation, version 0.1\n");
  }

private:
  std::string_view m_program_name;
  CommandLineOptions &m_options;
};

/// Reads the full content of a file at the given path.
/// If, for some reason, we fail to read the file content then
/// std::nullopt is returned.
std::optional<std::string> read_file(std::string_view path) {
  constexpr std::size_t BUFFER_SIZE = 4096;
  char buffer[BUFFER_SIZE] = {0};

  auto stream = std::ifstream(path.data());
  if (!stream)
    return std::nullopt;

  std::string content;
  while (stream.read(buffer, BUFFER_SIZE)) {
    content.append(buffer, 0, stream.gcount());
  }

  content.append(buffer, 0, stream.gcount());
  return content;
}

void compile_file(std::string_view file_name, std::string_view file_content) {
  DiagnosticContext ctx;
  ctx.set_file_info(file_name, file_content);

  Lexer lexer(file_content.data());
  Parser parser(ctx, lexer);
  Program program = parser.parse_program();
  ProgramPrinter printer;
  printer.print_program(program);
}

int main(int argc, const char *argv[]) {
  CommandLineOptions cmd_options;
  CommandLineParser cmd_parser(cmd_options);
  if (!cmd_parser.parse(argc, argv))
    return EXIT_FAILURE;

  for (const auto &input_file : cmd_options.input_files) {
    auto file_content = read_file(input_file);
    if (!file_content.has_value()) {
      std::cerr << error << "failed to read file `" << input_file << "'"
                << std::endl;
      continue;
    }

    compile_file(input_file, *file_content);
  }

  return EXIT_SUCCESS;
}
