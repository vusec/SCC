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
    std::cout << "\n[" << hiddenLines << " lines hidden]\n";
    return;
  }
}

void FancyProgramPrinter::writeImpl(std::string_view s) {
  if (isPrintingHiddenChars()) {
    std::cout << s;
    return;
  }
  for (const char c : s) {
    bool printChar = true;
    // Do the actual printing if we're past the start line.
    if (lineIndex >= startLineIndex) {
      // If the current line is past the end of the available space then
      // just remember we didn't print it and skip the rest of the printing
      // logic.
      if (linesPrinted >= lineLimit) {
        if (c == '\n')
          hiddenLines++;
        continue;
      }

      // Don't print the character unless it's the final new line.
      if (c == '\n')
        printChar = false;

      // We're past the right side of the available line space.
      if (columnsPrinted >= columnLimit) {
        printChar = false;
      }

      // Count the printed characters.
      // FIXME: This underestimates space with UTF-8.
      ++columnsPrinted;

      // We're at the beginning of the line, so print the line number first.
      if (atStartOfLine) {
        atStartOfLine = false;
        // Don't print a newline in the first line.
        if (lineIndex > 1)
          std::cout << "\n";
        printLinePrefix(lineIndex);
      }
      // Print the actual character.
      if (printChar)
        std::cout << c;

      if (c == '\n')
        linesPrinted++;
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
