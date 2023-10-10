#include "report.hpp"

#include <algorithm>
#include <cassert>
#include <iostream>
#include <vector>

/* --------------------------------------------------------
 * class ReportColorGenerator
 */

ReportColor ReportColorGenerator::next_color() {
  constexpr uint32_t COLOR_COUNT = 6 /* excluding color NONE */;
  m_current_idx = (m_current_idx + 1) % COLOR_COUNT;
  return static_cast<ReportColor>(m_current_idx + 1 /* excluding color NONE */);
}

/* --------------------------------------------------------
 * class ReportConsolePrinter
 */

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
    /// The color for the source code box.
    const char *box = "37";
    /// The color for the locus (filename:line:column).
    const char *locus = "0";
    /// The color for the line number.
    const char *line_number = "0";
  };

  explicit ReportConsolePrinter(std::ostream &out) : m_out(out) {}

  /// Prints the given report to stdout.
  void print(const Report &report) {
    print_message(report.severity, report.code, report.message);

    if (!report.location.has_value()) {
      return;
    }

    uint32_t line_number, column_number;
    report.manager.resolve_source_location(report.location.value(), line_number,
                                           column_number);

    const auto source_line = report.manager.get_line_at(line_number);

    print_source_code_header(report.manager.get_file_name(), line_number, column_number);
    print_source_line(line_number, source_line);
    print_spans(report.spans);
    print_source_code_footer(report.note);
  }

private:
  /// A trivial labelled span that only spans a single line of code.
  struct TrivialLabelledSpan {
    std::string_view label;
    std::string_view color;
    uint32_t start = 0;  // 0-indexed, relative to the start of the line
    uint32_t length = 0; // byte length

    /// Returns true if the span is empty that is if its length is null.
    [[nodiscard]] inline bool is_empty() const { return length == 0; }
    /// Returns true if the span's label is non empty.
    [[nodiscard]] inline bool has_label() const { return !label.empty(); }
  };

  void print_message(ReportSeverity severity,
                     const std::optional<uint32_t> &code,
                     std::string_view message) {

    const char *severity_color;
    const char *severity_name;
    char severity_prefix;
    switch (severity) {
    case ReportSeverity::WARNING:
      severity_color = m_colors.warning;
      severity_name = "warning";
      severity_prefix = 'W';
      break;
    case ReportSeverity::ERROR:
      severity_color = m_colors.error;
      severity_name = "error";
      severity_prefix = 'E';
      break;
    }

    if (code.has_value())
      print_colored(severity_color, "{}[{}{:04}]:", severity_name,
                    severity_prefix, code.value());
    else
      print_colored(severity_color, "{}:", severity_name);

    m_out << " " << message << "\n";
  }

  void print_source_code_header(std::string_view file_name,
                                uint32_t line_number, uint32_t column_number) {
    m_out << "     ";
    print_colored(m_colors.box, "╭─[");
    print_colored(m_colors.locus, "{}:{}:{}", file_name, line_number,
                  column_number);
    print_colored(m_colors.box, "]");
    m_out << "\n";
  }

  void print_source_line(uint32_t line_number, std::string_view source_line) {
    print_colored(m_colors.line_number, "{:4}", line_number);
    m_out << " ";
    print_colored(m_colors.box, "│");
    m_out << " " << source_line << "\n";
  }

  void print_span_margin() {
    m_out << "     ";
    print_colored(m_colors.box, "·");
    m_out << " ";
  }

  void print_underlines(const std::vector<TrivialLabelledSpan> &spans) {
    print_span_margin();

    std::string buffer;

    size_t last_column_with_character = 0;
    for (const auto &span : spans) {
      size_t columns_to_fill = span.start - last_column_with_character;
      while (columns_to_fill--)
        buffer.push_back(' ');

      if (m_use_colors)
        buffer.append(span.color);

      size_t span_length = span.length - 1;
      while (span_length--)
        buffer.append("─");

      if (span.has_label())
        buffer.append("┬");
      else
        buffer.append("─");

      if (m_use_colors)
        buffer.append("\x1b[0m");

      last_column_with_character = span.start + span.length;
    }

    m_out << buffer << "\n";
  }

  void print_spans_label(const std::vector<TrivialLabelledSpan> &spans) {
    // All spans without label must have been removed before calling this
    // function.
    assert(std::count_if(spans.begin(), spans.end(), [](const auto &span) {
             return !span.has_label();
           }) == 0);

    for (size_t i = 0; i < spans.size(); ++i) {
      print_span_margin();

      std::string buffer;
      size_t last_column_with_character = 0;

      for (size_t j = 0; j < spans.size() - i; ++j) {
        size_t columns_to_fill =
            (spans[j].start + spans[j].length - 1) - last_column_with_character;
        while (columns_to_fill--)
          buffer.push_back(' ');

        if (m_use_colors)
          buffer.append(spans[j].color);

        bool is_last_span_in_row = j == spans.size() - i - 1;
        if (is_last_span_in_row) {
          buffer.append("╰─ ");
          buffer.append(spans[j].label);
        } else {
          buffer.append("│");
        }

        if (m_use_colors)
          buffer.append("\x1b[0m");

        last_column_with_character = spans[j].start + spans[j].length;
      }

      m_out << buffer << "\n";
    }
  }

  void print_spans(const std::vector<LabelledSpan> &spans) {
    std::vector<TrivialLabelledSpan> trivial_spans;

    for (const auto &span : spans) {
      TrivialLabelledSpan trivial_span;
      trivial_span.label = span.label;
      trivial_span.color = report_color_to_ansi(span.color);
      trivial_span.start = span.span.location.offset;
      trivial_span.length = span.span.length;
      trivial_spans.push_back(trivial_span);
    }

    // Sort spans by their starting position.
    std::sort(
        trivial_spans.begin(), trivial_spans.end(),
        [](const auto &lhs, const auto &rhs) { return lhs.start < rhs.start; });

    // Check for overlapping spans and try to correct them.
    uint32_t last_column = 0;
    for (auto &trivial_span : trivial_spans) {
      const uint32_t end_column = trivial_span.start + trivial_span.length;

      if (trivial_span.start < last_column) {
        trivial_span.start = last_column;
        if (end_column > trivial_span.start) {
          trivial_span.length = end_column - trivial_span.start;
        } else {
          trivial_span.length = 0;
        }
      }

      last_column = end_column;
    }

    // Remove empty spans (spans with a null length).
    std::erase_if(trivial_spans,
                  [](const auto &span) { return span.is_empty(); });

    if (trivial_spans.empty())
      return;

    print_underlines(trivial_spans);

    // Remove spans without label.
    std::erase_if(trivial_spans,
                  [](const auto &span) { return !span.has_label(); });
    print_spans_label(trivial_spans);
  }

  void print_source_code_footer(std::string_view note) {
    m_out << "     ";
    print_colored(m_colors.box, "╰─");

    if (!note.empty()) {
      m_out << " ";
      print_colored(m_colors.note, "note:");
      m_out << " " << note;
    }

    m_out << "\n";
  }

  /// Prints the given message (that will be formatted using std::format())
  /// with the given ANSI color (only the numerical value between the \x1b[ and
  /// m). If the use of colors is disabled, then the formatted message is
  /// printed verbatim.
  template <typename... Args>
  void print_colored(const char *ansi_color, std::string_view message,
                     Args &&...args) {
    if (m_use_colors)
      m_out << "\x1b[" << ansi_color << "m";
    m_out << fmt::vformat(message,
                          fmt::make_format_args(std::forward<Args>(args)...));
    if (m_use_colors)
      m_out << "\x1b[0m";
  }

  /// Returns the full ANSI escape code sequence corresponding to the given
  /// report color.
  [[nodiscard]] static std::string_view
  report_color_to_ansi(ReportColor color) {
    switch (color) {
    case ReportColor::RED:
      return "\x1b[31m";
    case ReportColor::GREEN:
      return "\x1b[32m";
    case ReportColor::YELLOW:
      return "\x1b[33m";
    case ReportColor::BLUE:
      return "\x1b[34m";
    case ReportColor::MAGENTA:
      return "\x1b[35m";
    case ReportColor::CYAN:
      return "\x1b[36m";
    case ReportColor::NONE:
    default:
      return "\x1b[0m";
    }
  }

private:
  std::ostream &m_out;
  // The ANSI colors to use to colorize the output.
  Colors m_colors;
  // Should we emit ANSI escape codes to colorize the output?
  bool m_use_colors = false;
};

void Report::print(std::ostream &out) const {
  ReportConsolePrinter printer(out);
  printer.print(*this);
}

/* --------------------------------------------------------
 * class ReportManager
 */

std::string_view ReportManager::get_line_at(uint32_t line_number) {
  fill_line_map_if_needed();

  uint32_t start_position = m_line_map.get_line_start_position(line_number);

  size_t line_length = 0;
  const char *begin = m_file_content.data() + start_position;
  const char *it = begin;
  while (*it != '\0' && *it != '\n' && *it != '\r') {
    ++line_length;
    ++it;
  }

  return {begin, line_length};
}

void ReportManager::resolve_source_location(SourceLocation location,
                                            uint32_t &line_number,
                                            uint32_t &column_number) {
  fill_line_map_if_needed();

  m_line_map.get_line_and_column_numbers(location.offset, line_number,
                                         column_number);
}

void ReportManager::fill_line_map_if_needed() {
  if (m_line_map_filled)
    return;

  m_line_map.prefill(m_file_content);
  m_line_map_filled = true;
}
