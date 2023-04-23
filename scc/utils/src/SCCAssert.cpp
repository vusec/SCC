#include "scc/utils/SCCAssert.h"
#include "scc/utils/OutStream.h"
#include <iostream>

void SCCAssertImpl(std::optional<std::string> lhsOrCond,
                   std::optional<std::string> rhs, std::string checkStr,
                   std::string msg, std::string file, std::string function,
                   int line, std::string extraInfo) {
  StdErrOutStream out;
  out.printColor(OutStream::BoldRed, "\nError: ");
  out.printColor(OutStream::Red, msg + "\n");

  out << "  in function ";
  out.printColor(OutStream::Green, file);
  out << ":";
  out.printColor(OutStream::Cyan, std::to_string(line));
  out << "\n";

  if (rhs) {
    std::cerr << "  comparison failed: " << checkStr << "\n";
    std::cerr << "    left side: " << *lhsOrCond << "\n";
    std::cerr << "   right side: " << *rhs << "\n";
  }
  if (!extraInfo.empty())
    std::cerr << "  Additional information: " << extraInfo << "\n";
  // Should already be flushed but let's be sure about this.
  std::cerr.flush();
  abort();
}
