#ifndef FANCYPROGRAMPRINTER_H
#define FANCYPROGRAMPRINTER_H

#include "scc/utils/OutStream.h"
#include <iostream>

/// Prints a program with syntax highlighting, truncated output and all the
/// other stuff that makes it pleasent to view.
struct FancyProgramPrinter : public OutStream {
  unsigned lineIndex = 1;
  unsigned lineLimit = 60;
  unsigned columnsPrinted = 0;
  unsigned columnLimit = 0;
  unsigned startLineIndex = 0;
  bool atStartOfLine = true;
  unsigned linesPrinted = 0;
  unsigned hiddenLines = 0;
  FancyProgramPrinter(unsigned limit, unsigned column, unsigned startLine);
  ~FancyProgramPrinter();

  virtual void writeImpl(std::string_view s);
};
#endif // FANCYPROGRAMPRINTER_H
