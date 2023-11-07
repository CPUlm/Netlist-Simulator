#ifndef NETLIST_REPORT_HPP
#define NETLIST_REPORT_HPP

#include "token.hpp"

#include <iostream>
#include <optional>
#include <sstream>
#include <iomanip>

class ReportContext;

enum class ReportSeverity { WARNING, ERROR, INTERRUPTED, };

struct Report {
  const ReportContext &context;
  ReportSeverity severity;
  std::optional<SourcePosition> position;
  std::optional<int> code;
  std::string message;
  std::string help;

  explicit Report(ReportSeverity severity, const ReportContext &context) noexcept
      : context(context), severity(severity) {}

  void print(std::ostream &out = std::cerr) const;
  [[noreturn]] void exit(std::ostream &out = std::cerr) const;
};

class ReportBuilder {
public:
  explicit ReportBuilder(ReportSeverity severity, const ReportContext &context) : m_report(severity, context) {}

  [[nodiscard]] ReportBuilder &with_location(const SourcePosition &position) noexcept {
    m_report.position = position;
    return *this;
  }

  /// Sets the main message for the report. All argument are concatenated.
  template<typename... T>
  [[nodiscard]] ReportBuilder &with_message(T... args) {
    std::stringstream sstr;
    ((sstr << args), ...);
    m_report.message = sstr.str();
    return *this;
  }

  /// Sets a help message for the report that adds additional information to how
  /// to use the command line program.
  [[nodiscard]] ReportBuilder &with_help(const std::string &help) {
    m_report.help = help;
    return *this;
  }

  /// Sets a code for the error or the warning.
  [[nodiscard]] ReportBuilder &with_code(unsigned char code) noexcept {
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
  explicit ReportContext(std::string filename, bool colored_output) noexcept:
      m_file_name(std::move(filename)), m_colored_output(colored_output) {}

  explicit ReportContext(bool colored_output) noexcept: m_file_name(), m_colored_output(colored_output) {}

  [[nodiscard]] ReportBuilder report(ReportSeverity severity) const {
    return ReportBuilder(severity, *this);
  }

  [[nodiscard]] std::string get_file_name() const {
    return m_file_name;
  }

  [[nodiscard]] bool colored_output() const { return m_colored_output; }

  [[nodiscard]] std::string get_location(std::optional<SourcePosition> pos) const noexcept;

private:
  const std::string m_file_name;
  bool m_colored_output;
};

#endif // NETLIST_REPORT_HPP
