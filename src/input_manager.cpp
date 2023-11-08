#include "input_manager.hpp"

#include "utilities.hpp"
#include "report.hpp"
#include "lexer.hpp"
#include "parser.hpp"

InputManager::InputManager(const std::vector<std::string_view> &input_files) {
  for (std::string_view f : input_files) {
    ReportContext ctx(std::string(f), true);
    std::string data_file = Utilities::read_file(ctx, f);
    Lexer l(ctx, data_file.data());
    Parser p(ctx, l);

    for (auto const &[block_name, data] : p.parse_input()) {
      memory_blocks.emplace(std::piecewise_construct,
                            std::forward_as_tuple(block_name),
                            std::forward_as_tuple(data));
    }
  }
}
