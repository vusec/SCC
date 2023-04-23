#ifndef PRINTSTATE_H
#define PRINTSTATE_H

#include "scc/utils/Maybe.h"
#include "scc/utils/OutStream.h"

#include <unordered_set>
#include <vector>

class Program;

/// Stores information used when printing the textual form of a program.
struct PrintState {

  /// Creates a new print state for printing the given program to the given
  /// stream.
  PrintState(const Program &p, OutStream &out) : prog(p), out(out) {}

  /// Prints an include to the stream.
  ///
  /// If the header has already been included, nothing is printed.
  void printHeaderInclude(std::string name);

  /// Return the program that is currently being printed.
  const Program &getProgram() { return prog; }

  /// Returns the outpust ream.
  OutStream &getOut() const { return out; }

private:
  /// The program being printed.
  const Program &prog;
  /// The stream that is the output sent to.
  OutStream &out;

  /// External headers that have already been included in the printing process.
  std::unordered_set<std::string> includedExternalHeaders;
};

#endif // PRINTSTATE_H
