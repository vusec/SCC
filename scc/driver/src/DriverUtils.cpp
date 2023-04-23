#include "scc/driver/DriverUtils.h"
#include <filesystem>
#include <iostream>
#include <unistd.h>

void DriverUtils::clearScreen() {
  // Clear screen and move cursor to top left.
  std::cout << "\033[2J\033[H";
  std::cout.flush();
}

bool DriverUtils::isStdoutAProperTerminal() {
  if (const char *term = ::getenv("TERM"))
    if (std::string(term) == "dumb")
      return false;
  return isatty(STDIN_FILENO);
}

const std::string DriverUtils::pythonPathVar = "PYTHONPATH";

std::string DriverUtils::prependPythonPath(std::string argv0) {
  using path = std::filesystem::path;

  // Find the python library relative to our argv0 binary.
  const path sccPath = argv0;
  const path binDir = sccPath.parent_path();
  const path baseDir = binDir.parent_path();
  path sccPythonlibs = baseDir / path("runtime") / path("python");

  // Check if PYTHONPATH is already set.
  std::string existingPythonPath;
  if (const char *existing = ::getenv(pythonPathVar.c_str())) {
    existingPythonPath = existing;
    // Add the seperator between our new path and the existing.
    existingPythonPath = ":" + existingPythonPath;
  }

  // Set the new PYTHONPATH.
  std::string newPythonPath = sccPythonlibs.string() + existingPythonPath;
  const bool replace = true;
  ::setenv(pythonPathVar.c_str(), newPythonPath.c_str(), replace);

  return newPythonPath;
}
