#ifndef DRAW_TOOLS_H
#define DRAW_TOOLS_H

#include <string>
#include <vector>

/// Utilities for drawing a terminal UI.
struct DrawTools {

  /// Draws a header in our boxy UI.
  static void printHeader(std::string header, std::string first,
                          std::string last);

  /// Writes a text surrounded by left/right borders.
  static void printLine(std::string info);

  /// Draws the top bar of the fuzzer with the available views.
  static void drawTopBar(const std::vector<std::string> headers,
                         std::size_t activeIndex);
};

#endif
