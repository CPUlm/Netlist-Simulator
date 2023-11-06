#include "dependency_graph.hpp"
#include "disassembler.hpp"
#include "driver/command_line_parser.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "simulator/simulator.hpp"

#include <charconv>
#include <chrono>
#include <fstream>
#include <iostream>

[[nodiscard]] std::string read_file(ReportManager &report_manager, std::string_view path) {
  // From https://stackoverflow.com/a/116220/8959979

  try {
    constexpr auto read_size = std::size_t(4096);
    auto stream = std::ifstream(path.data());
    stream.exceptions(std::ios_base::badbit);

    if (!stream)
      throw std::ios_base::failure("file does not exist");

    auto out = std::string();
    auto buf = std::string(read_size, '\0');
    while (stream.read(&buf[0], read_size)) {
      out.append(buf, 0, stream.gcount());
    }
    out.append(buf, 0, stream.gcount());
    return out;
  } catch (const std::exception &) {
    report_manager.report(ReportSeverity::ERROR).with_message("failed to read file `{}'", path).finish().exit();
    return {}; // unreachable
  }
}

[[nodiscard]] std::string format_duration(const std::chrono::duration<double> &dur) {
  if (dur.count() < 1e-3) {
    return fmt::format("{} ns", dur.count() * 1000000.0);
  } else if (dur.count() < 1.0) {
    return fmt::format("{} ms", dur.count() * 1000.0);
  } else {
    return fmt::format("{} s", dur.count());
  }
}

static void query_program_inputs(ReportManager &report_manager, Simulator &simulator) {
  const auto program = simulator.get_program();
  if (!program->has_inputs())
    return;

  for (const auto input_reg : program->get_inputs()) {
    bool has_error;
    do {
      fmt::print("{} ? ", program->get_register_name(input_reg));
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
}

static void print_program_outputs(const Simulator &simulator) {
  const auto program = simulator.get_program();
  if (!program->has_outputs())
    return;

  for (const auto output : program->get_outputs()) {
    const auto bus_size = program->registers[output.index].bus_size;
    fmt::println("=> {} = {:0{}b}", program->get_register_name(output), simulator.get_register(output), bus_size);
  }
}

static void simulate_cycles(ReportManager &report_manager, Simulator &simulator, size_t cycles, bool timeit) {
  unsigned step = 1;
  while (cycles == 0 || step <= cycles) {
    fmt::println("Step {}:", step);

    query_program_inputs(report_manager, simulator);

    const auto start = std::chrono::high_resolution_clock::now();
    simulator.cycle();
    const auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> dur = end - start;
    step += 1;

    print_program_outputs(simulator);

    if (timeit)
      fmt::println("The simulation took {}", format_duration(dur));
  }
}

static void simulate_cycles_fast(ReportManager &report_manager, Simulator &simulator, size_t cycles, bool timeit) {
  const auto start = std::chrono::high_resolution_clock::now();
  simulator.simulate(cycles);
  const auto end = std::chrono::high_resolution_clock::now();
  const std::chrono::duration<double> dur = end - start;

  print_program_outputs(simulator);

  if (timeit)
    fmt::println("The simulation took {}", format_duration(dur));
}

int main(int argc, const char *argv[]) {
  // I think that fmt::print() uses stdio.h internally but in some other parts
  // we use std::cout. So to be sure, we request synchronization.
  std::ostream::sync_with_stdio();

  ReportManager report_manager;
  CommandLineParser cmd_parser(report_manager, argc, argv);
  const auto options = cmd_parser.parse();

  std::string source_code = read_file(report_manager, options.input_file);
  report_manager.register_file_info(options.input_file, source_code);

  Lexer lexer(report_manager, source_code.data());
  Parser parser(report_manager, lexer);
  std::shared_ptr<Program> program = parser.parse_program();
  assert(program != nullptr); // if we failed to parse the program then we should have exited before.

  if (options.syntax_only)
    return EXIT_SUCCESS;

  DependencyGraph graph = DependencyGraph::build(program);
  if (options.dependency_graph) {
    graph.dump_dot();
    return EXIT_SUCCESS;
  }

  graph.schedule(report_manager);
  if (options.schedule) {
    Disassembler::disassemble(program);
    return EXIT_SUCCESS;
  }

  Simulator simulator(program);
  if (options.fast) {
    simulate_cycles_fast(report_manager, simulator, options.cycles, options.timeit);
  } else {
    simulate_cycles(report_manager, simulator, options.cycles, options.timeit);
  }

  return EXIT_SUCCESS;
}
