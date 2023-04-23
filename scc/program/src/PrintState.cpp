#include "scc/program/PrintState.h"

void PrintState::printHeaderInclude(std::string name) {
  if (includedExternalHeaders.count(name) != 0)
    return;
  includedExternalHeaders.insert(name);

  out.setColor(OutStream::Color::Cyan);
  out << "#include <" << name << ">\n";
  out.setColor(OutStream::Color::Default);
}
