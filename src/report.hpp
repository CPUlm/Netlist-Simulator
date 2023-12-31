#ifndef NETLIST_REPORT_HPP
#define NETLIST_REPORT_HPP

#include "line_map.hpp"
#include "token.hpp"

#include <cstdint>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

#include <fmt/format.h>

/// \addtogroup report The Report API
/// All the mechanisms to report errors and warnings to the user.
/// @{

/// \brief All the different supported colors for spans in reports.
enum class ReportColor {
  NONE,
  RED,
  GREEN,
  YELLOW,
  BLUE,
  MAGENTA,
  CYAN,
};

/// \brief Utility class to generate different colors for the spans of a report.
class ReportColorGenerator {
public:
  /// Returns the next unused color. Once all colors were used at least once,
  /// the generator loop on itself and previous used colors may be returned
  /// again.
  ReportColor next_color();

private:
  uint32_t m_current_idx = 0;
};

/// \brief A source code span with an optional label and color.
struct LabelledSpan {
  /// Can be empty if there is no label.
  std::string label;
  SourceRange span = {};
  ReportColor color = ReportColor::NONE;
};

/// \brief The different report severities supported.
enum class ReportSeverity { WARNING, ERROR };

class ReportManager;

/// \brief A single report.
///
/// You should never create a report yourself. Instead, use directly the ReportManager
/// class and the ReportManager::report() method which returns an instance of a
/// ReportBuilder. Once the report is build, you can either ignore it, call print()
/// or exit() depending on what you want.
///
/// \see ReportBuilder and ReportManager
struct Report {
  ReportManager &manager;
  ReportSeverity severity = ReportSeverity::ERROR;
  std::optional<SourceLocation> location;
  std::optional<std::uint32_t> code;
  std::string message;
  std::string note;
  std::vector<LabelledSpan> spans;

  explicit Report(ReportManager &manager) : manager(manager) {}

  /// Prints the report to the given output stream.
  void print(std::ostream &out = std::cerr) const;

  /// Same as exit(int) but use the report code as the exit code if any.
  /// Otherwise, 1 is used as the exit code.
  void exit();
  /// Prints the report to the standard error output and then exists the
  /// program with the given \a error_code.
  void exit(int error_code);
};

/// \brief An utility class to help building a report.
///
/// As for Report, you should never create yourself an instance of this class.
/// Instead, use the ReportManager class and the ReportManager::report() method.
///
/// \see ReportManager
class ReportBuilder {
public:
  explicit ReportBuilder(ReportSeverity severity, ReportManager &m_manager)
      : m_report(m_manager) {
    m_report.severity = severity;
  }

  ReportBuilder &with_location(SourceLocation location) {
    m_report.location = location;
    return *this;
  }

  /// Sets the main message for the report.
  ///
  /// The message is formatted using std::format() and therefore the syntax and
  /// arguments supported by std::format() are also by this function.
  template <typename... T>
  ReportBuilder &with_message(fmt::format_string<T...> format, T &&...args) {
    m_report.message = fmt::vformat(format, fmt::make_format_args(args...));
    return *this;
  }

  /// Sets a note message for the report that adds additional information (maybe
  /// also a hint to fix the error).
  ///
  /// The message is formatted using std::format() and therefore the syntax and
  /// arguments supported by std::format() are also by this function.
  ///
  /// Before:
  /// ```
  /// error: the variable 'foi' is unknown
  ///     ╭─[file:1:1]
  ///   1 │ foi
  ///     ╰─
  /// ```
  ///
  /// After `with_note("did you mean '{}'", "foo")`:
  /// ```
  /// error: the variable 'foi' is unknown
  ///     ╭─[file:1:1]
  ///   1 │ foi
  ///     ╰─ note: did you mean 'foo'
  /// ```
  template <typename... T>
  ReportBuilder &with_note(fmt::format_string<T...> format, T &&...args) {
    m_report.note = fmt::vformat(format, fmt::make_format_args(args...));
    return *this;
  }

  /// Sets a code for the error or the warning.
  ///
  /// Before:
  /// ```
  /// error: the variable 'foi' is unknown
  /// ```
  ///
  /// After `with_code(42)`:
  /// ```
  /// error[E0042]: the variable 'foi' is unknown
  /// ```
  ReportBuilder &with_code(uint32_t code) {
    m_report.code = code;
    return *this;
  }

  /// Adds a unlabelled span for the report.
  ///
  /// Before:
  /// ```
  /// error: the variable 'foi' is unknown
  ///     ╭─[file:1:1]
  ///   1 │ foi
  ///     ╰─
  /// ```
  ///
  /// After `with_span(/* span of the identifier 'foi' */)`:
  /// ```
  /// error: the variable 'foi' is unknown
  ///     ╭─[file:1:1]
  ///   1 │ foi
  ///     · ───
  ///     ╰─
  /// ```
  ReportBuilder &with_span(SourceRange span) {
    LabelledSpan labelled_span;
    labelled_span.span = span;
    labelled_span.color = m_color_generator.next_color();
    m_report.spans.push_back(labelled_span);
    return *this;
  }

  /// Adds a labelled span for the report.
  ///
  /// The label is formatted using std::format() and therefore the syntax and
  /// arguments supported by std::format() are also by this function.
  ///
  /// Before:
  /// ```
  /// error: the variable 'foi' is unknown
  ///     ╭─[file:1:1]
  ///   1 │ foi
  ///     ╰─
  /// ```
  ///
  /// After `with_span(/* span of the identifier 'foi' */, "did you mean '{}'",
  /// "foo")`:
  /// ```
  /// error: the variable 'foi' is unknown
  ///     ╭─[file:1:1]
  ///   1 │ foi
  ///     · ──┬
  ///     ·   ╰─ did you mean 'foo'
  ///     ╰─
  /// ```
  template <typename... T>
  ReportBuilder &with_span(SourceRange span, fmt::format_string<T...> format,
                           T &&...args) {
    LabelledSpan labelled_span;
    labelled_span.label = fmt::vformat(format, fmt::make_format_args(args...));
    labelled_span.span = span;
    labelled_span.color = m_color_generator.next_color();
    m_report.spans.push_back(labelled_span);
    return *this;
  }

  /// Builds the report with all information previously given to the builder.
  ///
  /// The report is not yet printed, you must call Report::print() for that.
  Report finish() { return m_report; }

private:
  ReportColorGenerator m_color_generator;
  Report m_report;
};

/// \brief The main class for the report mechanism.
class ReportManager {
public:
  void register_file_info(std::string_view file_name,
                          std::string_view file_content) {
    m_file_name = file_name;
    m_file_content = file_content;
    m_line_map.clear();
    m_line_map_filled = false;
  }

  ReportBuilder report(ReportSeverity severity) {
    return ReportBuilder(severity, *this);
  }

  /// Gets the text of the requested line (1-numbered) for the current source
  /// file.
  [[nodiscard]] std::string_view get_line_at(uint32_t line_number);
  /// Maps the given source location into a line number and a column number.
  void resolve_source_location(SourceLocation location, uint32_t &line_number,
                               uint32_t &column_number);
  [[nodiscard]] std::string_view get_file_name() const { return m_file_name; }

private:
  /// Fills the line map if it is not already.
  void fill_line_map_if_needed();

private:
  std::string_view m_file_name;
  std::string_view m_file_content;
  LineMap m_line_map;
  bool m_line_map_filled;
};

/// @}

#endif // NETLIST_REPORT_HPP
