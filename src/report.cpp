#include "report.hpp"

#include <iostream>
#include <vector>

std::string ReportContext::get_location(std::optional<SourcePosition> pos) const noexcept {
  if (pos.has_value()) {
    return fmt::format("{}:{}:{}", get_file_name(), pos->line, pos->begin);
  } else {
    return std::string(get_file_name());
  }
}

/// Utility class implementing the report's console printer.
class ReportConsolePrinter {
public:
  struct Colors {
    /// The color for error messages.
    const char *error = "1;31";
    /// The color for warning messages.
    const char *warning = "1;33";
    /// The color for notes.
    const char *note = "1;34";
    /// The color for the text.
    const char *text = "0";
    /// The color for the locus (filename:line:column).
    const char *locus = "0";
  };

  explicit ReportConsolePrinter(std::ostream &out) : m_out(out) {}

  /// Prints the given report to stdout.
  void print(const Report &report) {
    print_colored(report.context, m_colors.text, "In file ");
    print_colored(report.context, m_colors.locus,
                  report.context.get_location(report.position));
    print_colored(report.context, m_colors.text, ":\n");
    print_message(report.context, report.severity, report.code, report.message);

    if (!report.note.empty()) {
      print_colored(report.context, m_colors.note, "note:");
      print_colored(report.context, m_colors.text, " {}\n", report.note);
    }
  }

private:
  void print_message(ReportContext context, ReportSeverity severity,
                     const std::optional<uint32_t> &code,
                     std::string_view message) {

    const char *severity_color;
    const char *severity_name;
    char severity_prefix;

    switch (severity) {
    case ReportSeverity::WARNING: //
      severity_color = m_colors.warning;
      severity_name = "warning";
      severity_prefix = 'W';
      break;
    case ReportSeverity::ERROR: //
      severity_color = m_colors.error;
      severity_name = "error";
      severity_prefix = 'E';
      break;
    }

    if (code.has_value()) {
      print_colored(context, severity_color, "{}[{}{:04}]:", severity_name,
                    severity_prefix, code.value());
    } else {
      print_colored(context, severity_color, "{}:", severity_name);
    }

    print_colored(context, m_colors.text, " {}\n", message);
  }

  /// Prints the given message (that will be formatted using std::format())
  /// with the given ANSI color (only the numerical value between the \x1b[ and
  /// m). If the use of colors is disabled, then the formatted message is
  /// printed verbatim.
  template<typename... Args>
  void print_colored(ReportContext c, const char *ansi_color,
                     std::string_view message, Args &&...args) {
    if (c.colored_output())
      m_out << "\x1b[" << ansi_color << "m";
    m_out << fmt::vformat(message,
                          fmt::make_format_args(std::forward<Args>(args)...));
    if (c.colored_output())
      m_out << "\x1b[0m";
  }

private:
  std::ostream &m_out;
  // The ANSI colors to use to colorize the output.
  Colors m_colors;
};

void Report::print(std::ostream &out) const {
  ReportConsolePrinter printer(out);
  printer.print(*this);
}

[[noreturn]] void Report::exit(std::ostream &out) const {
  print(out);

  if (code.has_value()) {
    ::exit(code.value());
  } else {
    ::exit(1);
  }
}
