#ifndef GENERATORUTILS_H
#define GENERATORUTILS_H

#include <scc/program/Program.h>

/// Set of generic utilities for generating/mutating programs.
class GeneratorUtils {
public:
  /// Adds a empty main function to the program and returns it.
  static Function *addMain(Program &p);
};

#endif // GENERATORUTILS_H
