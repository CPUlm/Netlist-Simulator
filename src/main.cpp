#include <csignal>

#include <iostream>

#include "command_line_parser.hpp"
#include "utilities.hpp"

#include "lexer.hpp"
#include "parser.hpp"

#include "program_printer.hpp"
#include "dot_printer.hpp"
#include "scheduler.hpp"
#include "simulator.hpp"

void signal_handler([[maybe_unused]] int signal) {
  throw ExitProgramNow();
}

int main(int argc, const char *argv[]) {
  std::signal(SIGINT, signal_handler);
  std::signal(SIGTERM, signal_handler);

  CommandLineParser cmd_parser(argc, argv);

  if (cmd_parser.get_action() == NoAction) {
    return EXIT_SUCCESS;
  }

  ReportContext ctx(cmd_parser.get_netlist_file(), true);
  std::string netlist_content = Utilities::read_file(ctx, cmd_parser.get_netlist_file());
  Lexer lexer(ctx, netlist_content.data());
  Parser parser(ctx, lexer);
  Program::ptr program = parser.parse_program();

  switch (cmd_parser.get_action()) {
  case Simulate: {
    InputManager im(cmd_parser.get_inputs());
    Simulator s(ctx, im, program);
    size_t cycle_id = 0;
    try {

      if (cmd_parser.cycle_amount_defined()) {
        for (; cycle_id < cmd_parser.get_cycle_amount(); ++cycle_id) {
          if (cmd_parser.is_verbose()) {
            std::cout << "Step " << cycle_id + 1 << ":\n";
          }
          s.cycle();
          if (cmd_parser.is_verbose()) {
            s.print_outputs(std::cout);
            std::cout << "\n";
          }
        }
      } else {
        for (;;) {
          if (cmd_parser.is_verbose()) {
            std::cout << "Step " << cycle_id + 1 << ":\n";
          }
          s.cycle();
          if (cmd_parser.is_verbose()) {
            s.print_outputs(std::cout);
            std::cout << "\n";
          }
          cycle_id++;
        }
      }
      if (!cmd_parser.is_verbose()) {
        std::cout << "Step " << cycle_id << ":\n";
        s.print_outputs(std::cout);
        std::cout << "\n";
      }
    } catch (const ExitProgramNow &ignored) {
      std::cout << std::endl;
    }

    break;
  }

  case DotExport: {
    DotPrinter printer(program, std::cout);
    printer.print();
    break;
  }

  case PrintFile: {
    ProgramPrinter printer(program, std::cout);
    printer.print();
    break;
  }

  case Schedule: {
    bool is_first = true;

    for (const auto &v : Scheduler::schedule(ctx, program)) {
      if (is_first) {
        is_first = false;
      } else {
        std::cout << " -> ";
      }
      std::cout << v->get_name();
    }
    std::cout << "\n";
    break;
  }
  case NoAction:
    break;
  }

  return EXIT_SUCCESS;
}
