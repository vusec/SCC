#include "scc/driver/FancyProgramPrinter.h"
#include "scc/driver/ColorOutput.h"

static std::string getLinePrefix(unsigned line) {
  const size_t padTo = 3;

  std::string res;
  res.resize(padTo, ' ');
  res += std::to_string(line);
  while (res.size() > padTo)
    res = res.substr(1);
  return res;
}

static void printLinePrefix(unsigned line) {
  ColorOutput::print(ColorOutput::BoldCyan, getLinePrefix(line));
  ColorOutput::print(ColorOutput::Blue, "│");
}

FancyProgramPrinter::FancyProgramPrinter(unsigned limit, unsigned column,
                                         unsigned startLine)
    : lineLimit(limit), columnLimit(column), startLineIndex(startLine) {
  supportsColor = true;
  isInformal = true;
  columnLimit -= 5;
}

FancyProgramPrinter::~FancyProgramPrinter() {
  if (hiddenLines > 0) {
    std::cout << "[" << hiddenLines << " lines hidden]\n";
    return;
  }
}

void FancyProgramPrinter::writeImpl(std::string_view s) {
  for (const char c : s) {
    bool printChar = true;
    // Do the actual printing if we're past the start line.
    if (lineIndex >= startLineIndex) {
      // We're past the right side of the available line space.
      if (columnsPrinted >= columnLimit) {
        // Don't print the character unless it's either a hidden character
        // or the final new line.
        if (!isPrintingHiddenChars() || c != '\n')
          printChar = false;
      }

      // If the current line is past the end of the available space then
      // just remember we didn't print it and skip the rest of the printing
      // logic.
      if (linesPrinted >= lineLimit) {
        if (c == '\n')
          hiddenLines++;
        continue;
      }

      // Count the printed characters that aren't hidden.
      // FIXME: This underestimates space with UTF-8.
      if (!isPrintingHiddenChars())
        ++columnsPrinted;

      // We're at the beginning of the line, so print the line number first.
      if (atStartOfLine) {
        atStartOfLine = false;
        printLinePrefix(lineIndex);
      }
      // Print the actual character.
      if (printChar) {
        std::cout << c;
        if (c == '\n')
          linesPrinted++;
      }
    }
    // We printed a new line so remember that we need to print line number
    // before the next character.
    if (c == '\n') {
      atStartOfLine = true;
      columnsPrinted = 0;
      ++lineIndex;
    }
  }
}
