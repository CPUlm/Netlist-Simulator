#include <gtest/gtest.h>

#include "report.hpp"

class ReportTest : public ::testing::Test {
public:
  ReportContext ctx = ReportContext("file", false);
};

static void clear_stream(std::stringstream &stream) {
  stream.str(std::string());
  stream.clear();
}

TEST_F(ReportTest, with_message) {
  std::stringstream stream;

  // Error
  ctx.report(ReportSeverity::ERROR)
      .with_message("foo {1} {0}", "baz", "bar")
      .build()
      .print(stream);
  EXPECT_EQ(stream.str(), "In file file:\nerror: foo bar baz\n");

  clear_stream(stream);

  // Error
  ctx.report(ReportSeverity::WARNING)
      .with_message("{:04} test", 3)
      .build()
      .print(stream);
  EXPECT_EQ(stream.str(), "In file file:\nwarning: 0003 test\n");
}

TEST_F(ReportTest, with_code) {
  std::stringstream stream;

  // Error
  ctx.report(ReportSeverity::ERROR)
      .with_code(42)
      .with_message("foobar")
      .build()
      .print(stream);
  EXPECT_EQ(stream.str(), "In file file:\nerror[E0042]: foobar\n");

  clear_stream(stream);

  // Warning
  ctx.report(ReportSeverity::WARNING)
      .with_code(42)
      .with_message("foobar")
      .build()
      .print(stream);
  EXPECT_EQ(stream.str(), "In file file:\nwarning[W0042]: foobar\n");
}

TEST_F(ReportTest, with_note) {
  std::stringstream stream;

  ctx.report(ReportSeverity::ERROR)
      .with_message("foobar")
      .with_note("did you mean '{}'", "foo")
      .build()
      .print(stream);
  EXPECT_EQ(stream.str(), "In file file:\nerror: foobar\nnote: did you mean 'foo'\n");
}

TEST_F(ReportTest, with_position) {
  std::stringstream stream;

  ctx.report(ReportSeverity::ERROR)
      .with_location({42, 5})
      .with_message("foobar")
      .with_note("did you mean '{}'", "foo")
      .build()
      .print(stream);
  EXPECT_EQ(stream.str(), "In file file:42:5:\nerror: foobar\nnote: did you mean 'foo'\n");
}
