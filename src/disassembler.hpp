#ifndef NETLIST_SRC_DISASSEMBLER_HPP
#define NETLIST_SRC_DISASSEMBLER_HPP

#include "program.hpp"

/// \brief The Netlist program disassembler.
///
/// This class takes a program and then outputs a textual representation
/// to the given output stream.
///
/// The output is intended to contain the maximum information and to be a valid
/// parseable Netlist program.
class Disassembler {
public:
  /// Disassembles a single instruction and prints it to std::cout.
  ///
  /// \param instruction The instruction to disassemble.
  /// \param context The instruction's parent program.
  static void disassemble(const Instruction *instruction, const std::shared_ptr<Program> &context);
  /// Disassembles a single instruction and prints to the given output stream.
  ///
  /// \param instruction The instruction to disassemble.
  /// \param context The instruction's parent program.
  /// \param out The output stream.
  static void disassemble(const Instruction *instruction, const std::shared_ptr<Program> &context, std::ostream &out);

  /// Disassembles the program and prints it to std::cout.
  static void disassemble(const std::shared_ptr<Program> &program);
  /// Disassembles the program and prints it to the given output stream.
  static void disassemble(const std::shared_ptr<Program> &program, std::ostream &out);
};

#endif // NETLIST_SRC_DISASSEMBLER_HPP
