#include "input_manager.hpp"
#include "report.hpp"

#include <optional>
#include <iostream>
#include <charconv>

[[nodiscard]] std::optional<value_t> parse_value(const char *data, size_t size, int base) {
  value_t v;

  auto [ptr, ec] = std::from_chars(data, data + size, v, base);
  if (ec != std::errc() || ptr != data + size) {
    return {};
  } else {
    return v;
  }
}

const ReportContext ctx(true);

value_t InputManager::get_input_value(const Variable::ptr &var) {
  for (;;) {
    std::cout << "Value of '" << var->get_name() << "' (bus size: " << var->get_bus_size() << "): ";
    std::string buf;
    std::cin >> buf;

    if (std::cin.fail()) {
      std::cout << "\n" << std::flush;
      ctx.report(ReportSeverity::INTERRUPTED)
          .with_message("Interrupted during IO")
          .with_code(62)
          .build()
          .exit();
    }

    std::optional<value_t> v;

    if (buf.size() > 1 && buf[0] == '0' && buf[1] == 'b') {
      // Binary Constant
      v = parse_value(&buf[2], buf.size() - 2, 2);
    } else if (buf.size() > 1 && buf[0] == '0' && buf[1] == 'd') {
      // Decimal Constant
      v = parse_value(&buf[2], buf.size() - 2, 10);
    } else if (buf.size() > 1 && buf[0] == '0' && buf[1] == 'x') {
      // Hexadecimal Constant
      v = parse_value(&buf[2], buf.size() - 2, 16);
    } else {
      // Binary Digits
      v = parse_value(buf.data(), buf.size(), 2);
    }

    if (!v.has_value()) {
      std::cout << "Wrong formatted constant.\n";
    } else if (v > Bus::max_value(var->get_bus_size())) {
      std::cout << "Constant '" << buf << "' is too large for this bus size (" << var->get_bus_size() << ").\n";
    } else {
      return v.value();
    }
  }
}
