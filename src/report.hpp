#ifndef NETLIST_REPORT_HPP
#define NETLIST_REPORT_HPP

#include "token.hpp"

#include <iostream>
#include <optional>
#include <string_view>

#include <fmt/format.h>
#include <filesystem>

class ReportContext;

enum class ReportSeverity { WARNING, ERROR };

struct Report {
  ReportContext &context;
  ReportSeverity severity;
  std::optional<SourcePosition> position;
  std::optional<int> code;
  std::string message;
  std::string note;

  explicit Report(ReportSeverity severity, ReportContext &context) noexcept
      : context(context), severity(severity) {}

  void print(std::ostream &out = std::cerr) const;
  [[noreturn]] void exit(std::ostream &out = std::cerr) const;
};

class ReportBuilder {
public:
  explicit ReportBuilder(ReportSeverity severity, ReportContext &context) : m_report(severity, context) {}

  [[nodiscard]] ReportBuilder &with_location(const SourcePosition &position) noexcept {
    m_report.position = position;
    return *this;
  }

  /// Sets the main message for the report.
  ///
  /// The message is formatted using std::format() and therefore the syntax and
  /// arguments supported by std::format() are also by this function.
  template<typename... T>
  [[nodiscard]] ReportBuilder &with_message(fmt::format_string<T...> format, T &&...args) {
    m_report.message = fmt::vformat(format, fmt::make_format_args(args...));
    return *this;
  }

  /// Sets a note message for the report that adds additional information (maybe
  /// also a hint to fix the error).
  ///
  /// The message is formatted using std::format() and therefore the syntax and
  /// arguments supported by std::format() are also by this function.
  template<typename... T>
  [[nodiscard]] ReportBuilder &with_note(fmt::format_string<T...> format, T &&...args) {
    m_report.note = fmt::vformat(format, fmt::make_format_args(args...));
    return *this;
  }

  /// Sets a code for the error or the warning.
  [[nodiscard]] ReportBuilder &with_code(int code) noexcept {
    m_report.code = code;
    return *this;
  }

  /// Builds the report with all information previously given to the builder.
  ///
  /// The report is not yet printed, you must call Report::print() for that.
  [[nodiscard]] const Report &build() const noexcept { return m_report; }

private:
  Report m_report;
};

class ReportContext {
public:
  explicit ReportContext(const std::filesystem::path &filename, bool colored_output) noexcept:
      m_file_name(filename), m_colored_output(colored_output) {}

  ReportBuilder report(ReportSeverity severity) {
    return ReportBuilder(severity, *this);
  }

  [[nodiscard]] std::string get_file_name() const { return m_file_name; }

  [[nodiscard]] bool colored_output() const { return m_colored_output; }

  [[nodiscard]] std::string get_location(std::optional<SourcePosition> pos) const noexcept;

private:
  const std::filesystem::path &m_file_name;
  bool m_colored_output;
};

#endif // NETLIST_REPORT_HPP
