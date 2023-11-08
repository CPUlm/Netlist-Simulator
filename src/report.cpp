#include "report.hpp"

std::string ReportContext::get_location(std::optional<SourcePosition> pos) const {
  if (pos.has_value()) {
    std::stringstream sstr;
    sstr << get_file_name() << ":" << pos->line << ":" << pos->begin;
    return sstr.str();
  } else {
    return get_file_name();
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
    const char *help = "1;34";
    /// The color for the text.
    const char *text = "1";
    /// The color for the locus (filename:line:column).
    const char *locus = "4";
  };

  explicit ReportConsolePrinter(std::ostream &out) : m_out(out) {}

  /// Prints the given report to stdout.
  void print(const Report &report) {
    if (!report.context.get_file_name().empty()) {
      // Error with a file name
      print_colored(report.context, m_colors.text, "In file ");
      print_colored(report.context, m_colors.locus, report.context.get_location(report.position));
      print_colored(report.context, m_colors.text, ":\n");
    }

    print_message(report.context, report.severity, report.code, report.message);

    if (!report.help.empty()) {
      print_colored(report.context, m_colors.help, "help: ");
      print_colored(report.context, m_colors.text, report.help);
      print_colored(report.context, m_colors.text, "\n");
    }
  }

private:
  void print_message(const ReportContext &context,
                     ReportSeverity severity,
                     std::optional<int> code,
                     std::string_view message) {

    const char *severity_color;
    const char *severity_name;
    const char *severity_prefix;

    switch (severity) {
    case ReportSeverity::WARNING:
      severity_color = m_colors.warning;
      severity_name = "warning";
      severity_prefix = "W";
      break;
    case ReportSeverity::ERROR:
      severity_color = m_colors.error;
      severity_name = "error";
      severity_prefix = "E";
      break;
    case ReportSeverity::INTERRUPTED:
      print_colored(context, m_colors.text, message);
      print_colored(context, m_colors.text, "\n");
      return;
    }

    if (code.has_value()) {
      print_colored(context, severity_color, severity_name);
      print_colored(context, severity_color, "[");
      print_colored(context, severity_color, severity_prefix);

      std::stringstream sstr;
      sstr << std::setfill('0') << std::setw(4) << code.value();
      print_colored(context, severity_color, sstr.str());

      print_colored(context, severity_color, "]:");
    } else {
      print_colored(context, severity_color, severity_name);
      print_colored(context, severity_color, ":");
    }

    print_colored(context, m_colors.text, " ");
    print_colored(context, m_colors.text, message);
    print_colored(context, m_colors.text, "\n");
  }

  /// Prints the given message (that will be formatted using std::format())
  /// with the given ANSI color (only the numerical value between the \x1b[ and
  /// m). If the use of colors is disabled, then the formatted message is
  /// printed verbatim.
  void print_colored(const ReportContext &c, const char *ansi_color, std::string_view message) {
    if (c.colored_output()) {
      m_out << "\x1b[" << ansi_color << "m";
    }

    m_out << message;

    if (c.colored_output()) {
      m_out << "\x1b[0m";
    }
  }

private:
  std::ostream &m_out;
  // The ANSI colors to use to colorize the output.
  Colors m_colors;
};

void Report::print(std::ostream &out) const {
  ReportConsolePrinter printer(out);
  printer.print(*this);

  // Flush Everything to have consistent output
  std::flush(out);
  std::flush(std::cout);
  std::flush(std::cerr);
}

[[noreturn]] void Report::exit(std::ostream &out) const {
  print(out);

  if (code.has_value()) {
    std::exit(code.value());
  } else {
    std::exit(1);
  }
}
