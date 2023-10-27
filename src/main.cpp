#include "lexer.hpp"
#include "parser.hpp"
#include "simulator/simulator.hpp"

#include <fstream>

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
  auto code = read_file(argv[1]);

  ReportManager report_manager;
  report_manager.register_file_info(argv[1], code);
  Lexer lexer(code.data());
  Parser parser(report_manager, lexer);
  std::shared_ptr<Program> program = parser.parse_program();

  if (program) {
    Disassembler::disassemble(*program);

    Simulator simulator(program);
    simulator.set_register({ 0 }, 1);
    simulator.set_register({ 1 }, 0);
    simulator.print_inputs();
    simulator.execute();
    simulator.print_outputs();
  }

  return EXIT_SUCCESS;
}
