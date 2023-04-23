#include "scc/driver/DrawTools.h"

#include "scc/driver/ColorOutput.h"
#include "scc/driver/DriverUtils.h"
#include <cstdlib>
#include <iostream>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>

static const ColorOutput::Color BoldFont = ColorOutput::BoldWhite;
static const ColorOutput::Color Border = ColorOutput::Blue;

void DrawTools::printHeader(std::string header, std::string first,
                            std::string last) {

  struct winsize w = DriverUtils::getTerminalSize();

  size_t front = 8;
  std::string prefix = first;
  for (unsigned i = 0; i < front - 1U; ++i)
    prefix += "━";
  ColorOutput::print(Border, prefix);
  ColorOutput::print(BoldFont, header);

  std::string suffix;
  for (size_t i = front + header.size() + 1; i < w.ws_col; ++i)
    suffix += "━";
  suffix += last;

  ColorOutput::print(Border, suffix);
  std::cout << "\n";
}

void DrawTools::printLine(std::string info) {
  ColorOutput::print(Border, "┃");
  winsize w = DriverUtils::getTerminalSize();
  if (w.ws_col <= 2)
    return;
  info.resize(w.ws_col - 2, ' ');
  std::cout << info;
  ColorOutput::print(Border, "┃\n");
}

void DrawTools::drawTopBar(const std::vector<std::string> headers,
                           std::size_t activeIndex) {
  unsigned spaceLeft = DriverUtils::getTerminalSize().ws_col;

  ColorOutput::print(Border, "┏");
  for (unsigned int i = 0; i < 3; ++i)
    ColorOutput::print(Border, "━");

  spaceLeft -= 4;

  for (std::size_t i = 0; i < headers.size(); ++i) {
    const std::string header =
        " " + std::to_string(i + 1) + " " + headers.at(i);
    spaceLeft -= header.size();
    if (i == activeIndex)
      ColorOutput::print(ColorOutput::BoldRed, header);
    else
      ColorOutput::print(Border, header);
  }

  for (unsigned i = 1; i < spaceLeft; ++i) {
    ColorOutput::print(Border, "━");
  }
  ColorOutput::print(Border, "┓\n");
}
