#include "scc/utils/OutStream.h"
#include <iostream>

OutStream::~OutStream() {}

void OutStream::setColor(Color c) {
  color = c;
  if (supportsColor) {
    printingHidden = true;
    switch (color) {
    case Color::Black:
      writeImpl("\033[30m");
      break;
    case Color::White:
      writeImpl("\033[37m");
      break;
    case Color::Red:
      writeImpl("\033[31m");
      break;
    case Color::Magenta:
      writeImpl("\033[35m");
      break;
    case Color::Blue:
      writeImpl("\033[34m");
      break;
    case Color::Green:
      writeImpl("\033[32m");
      break;
    case Color::Cyan:
      writeImpl("\033[36m");
      break;
    case Color::Yellow:
      writeImpl("\033[33m");
      break;
    case Color::BoldBlack:
      writeImpl("\033[1m\033[30m");
      break;
    case Color::BoldWhite:
      writeImpl("\033[1m\033[37m");
      break;
    case Color::BoldRed:
      writeImpl("\033[1m\033[31m");
      break;
    case Color::BoldMagenta:
      writeImpl("\033[1m\033[35m");
      break;
    case Color::BoldBlue:
      writeImpl("\033[1m\033[34m");
      break;
    case Color::BoldGreen:
      writeImpl("\033[1m\033[32m");
      break;
    case Color::BoldCyan:
      writeImpl("\033[1m\033[36m");
      break;
    case Color::BoldYellow:
      writeImpl("\033[1m\033[33m");
      break;
    case Color::Default:
      writeImpl("\033[0m");
      break;
    }
    printingHidden = false;
  }
}

// pin vtable.
OutString::~OutString() {}

// pin vtable.
HashStream::~HashStream() {}

StdErrOutStream::~StdErrOutStream() {}

void StdErrOutStream::writeImpl(std::string_view s) { std::cerr << s; }
