#include <gtest/gtest.h>

#include "report.hpp"

class ReportTest : public ::testing::Test {
public:
  ReportManager manager;

  void SetUp() override {
    manager.register_file_info("file.test", "foo bar\nx = ADD a b");
  }
};

static void clear_stream(std::stringstream &stream) {
  stream.str(std::string());
  stream.clear();
}

TEST_F(ReportTest, color_generator) {
  ReportColorGenerator colors;
  EXPECT_EQ(colors.next_color(), ReportColor::GREEN);
  EXPECT_EQ(colors.next_color(), ReportColor::YELLOW);
  EXPECT_EQ(colors.next_color(), ReportColor::BLUE);
  EXPECT_EQ(colors.next_color(), ReportColor::MAGENTA);
  EXPECT_EQ(colors.next_color(), ReportColor::CYAN);
  EXPECT_EQ(colors.next_color(), ReportColor::RED);
  EXPECT_EQ(colors.next_color(), ReportColor::GREEN); // we loop again
}

TEST_F(ReportTest, get_line_at) {
  EXPECT_EQ(manager.get_line_at(1), "foo bar");
  EXPECT_EQ(manager.get_line_at(2), "x = ADD a b");
}

TEST_F(ReportTest, with_message) {
  std::stringstream stream;

  // Error
  manager.report(ReportSeverity::ERROR)
      .with_message("foo {1} {0}", "baz", "bar")
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), "error: foo bar baz\n");

  clear_stream(stream);

  // Error
  manager.report(ReportSeverity::WARNING)
      .with_message("{:04} test", 3)
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), "warning: 0003 test\n");
}

TEST_F(ReportTest, with_code) {
  std::stringstream stream;

  // Error
  manager.report(ReportSeverity::ERROR)
      .with_code(42)
      .with_message("foobar")
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), "error[E0042]: foobar\n");

  clear_stream(stream);

  // Warning
  manager.report(ReportSeverity::WARNING)
      .with_code(42)
      .with_message("foobar")
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), "warning[W0042]: foobar\n");
}

TEST_F(ReportTest, with_note) {
  std::stringstream stream;

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_note("did you mean '{}'", "foo")
      .finish()
      .print(stream);
  const auto str = stream.str();
  EXPECT_NE(str.find("note: did you mean 'foo'\n"), std::string::npos) << str;
}

TEST_F(ReportTest, single_span_without_label) {
  std::stringstream stream;

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_span({0, 3})
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), R"(error: foobar
     ╭─[file.test:1:1]
   1 │ foo bar
     · ───
     ╰─
)");
}

TEST_F(ReportTest, single_span_with_label) {
  std::stringstream stream;

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_span({0, 3}, "baz")
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), R"(error: foobar
     ╭─[file.test:1:1]
   1 │ foo bar
     · ──┬
     ·   ╰─ baz
     ╰─
)");
}

TEST_F(ReportTest, multiple_spans_without_label) {
  std::stringstream stream;

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_span({0, 3})
      .with_span({4, 3})
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), R"(error: foobar
     ╭─[file.test:1:1]
   1 │ foo bar
     · ─── ───
     ╰─
)");
}

TEST_F(ReportTest, multiple_spans_with_label) {
  std::stringstream stream;

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_span({0, 3}, "foo '{}'", "bar")
      .with_span({4, 3}, "baz")
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), R"(error: foobar
     ╭─[file.test:1:1]
   1 │ foo bar
     · ──┬ ──┬
     ·   │   ╰─ baz
     ·   ╰─ foo 'bar'
     ╰─
)");
}

TEST_F(ReportTest, multiple_spans_single_label) {
  std::stringstream stream;

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_span({0, 3})
      .with_span({4, 3}, "baz")
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), R"(error: foobar
     ╭─[file.test:1:1]
   1 │ foo bar
     · ─── ──┬
     ·       ╰─ baz
     ╰─
)");

  clear_stream(stream);

  manager.report(ReportSeverity::ERROR)
      .with_location(SourceLocation { 0 })
      .with_message("foobar")
      .with_span({0, 3}, "foo '{}'", "bar")
      .with_span({4, 3})
      .finish()
      .print(stream);
  EXPECT_EQ(stream.str(), R"(error: foobar
     ╭─[file.test:1:1]
   1 │ foo bar
     · ──┬ ───
     ·   ╰─ foo 'bar'
     ╰─
)");
}
