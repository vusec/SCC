#include "scc/driver/ColorOutput.h"

#include <iostream>

void ColorOutput::print(Color c, std::string s) {
  startColor(c);
  std::cout << s;
  std::cout << "\033[0m";
}

void ColorOutput::startColor(Color c) {
  switch (c) {
  case Black:
    printf("%s", "\033[30m");
    break;
  case Red:
    printf("%s", "\033[31m");
    break;
  case Green:
    printf("%s", "\033[32m");
    break;
  case yellow:
    printf("%s", "\033[33m");
    break;
  case Blue:
    printf("%s", "\033[34m");
    break;
  case Magenta:
    printf("%s", "\033[35m");
    break;
  case Cyan:
    printf("%s", "\033[36m");
    break;
  case White:
    printf("%s", "\033[37m");
    break;
  case BoldBlack:
    printf("%s", "\033[1m\033[30m");
    break;
  case BoldRed:
    printf("%s", "\033[1m\033[31m");
    break;
  case BoldGreen:
    printf("%s", "\033[1m\033[32m");
    break;
  case BoldYellow:
    printf("%s", "\033[1m\033[33m");
    break;
  case BoldBlue:
    printf("%s", "\033[1m\033[34m");
    break;
  case BoldMagenta:
    printf("%s", "\033[1m\033[35m");
    break;
  case BoldCyan:
    printf("%s", "\033[1m\033[36m");
    break;
  case BoldWhite:
    printf("%s", "\033[1m\033[37m");
    break;
  }
}
