#ifndef DRIVERUTILS_H
#define DRIVERUTILS_H

#include "scc/program/Program.h"
#include "scc/utils/OutStream.h"

#include <chrono>
#include <fstream>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

struct DriverUtils {
  /// Returns true if this is a terminal size that makes any sense to have
  /// an UI with.
  static bool hasViableWinSize() {
    winsize s = getTerminalSize();
    if (s.ws_col < 20 && s.ws_row < 20)
      return false;
    if (!isatty(0))
      return false;
    return true;
  }

  /// Returns the size of the current terminal.
  static winsize getTerminalSize() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w;
  }

  /// Clears the terminal screen of all contents.
  static void clearScreen();

  /// Returns true if stdout is a real tty that isn't 'dumb'.
  static bool isStdoutAProperTerminal();

  struct FileCleanup {
    std::string path;
    FileCleanup(std::string path) : path(path) {}
    ~FileCleanup() { std::remove(path.c_str()); }
  };

  /// A simple timer class.
  struct Timer {
    std::chrono::high_resolution_clock::time_point start;
    size_t &outTime;
    Timer(size_t &millis) : outTime(millis) {
      start = std::chrono::high_resolution_clock::now();
    }
    ~Timer() {
      using namespace std::chrono;
      auto now = high_resolution_clock::now();
      outTime += duration_cast<milliseconds>(now - start).count();
    }
  };

  /// File output stream.
  struct FileOut : public OutStream {
    std::ofstream stream;
    explicit FileOut(std::string path) : stream(path) {}
    virtual void writeImpl(std::string_view s) { stream << s; }
  };

  /// The environment variable of python library search path ("PYTHONPATH").
  static const std::string pythonPathVar;

  /// Prepends the right Python library path given the argv0 of the binary.
  static std::string prependPythonPath(std::string argv);

  /// Gets the file extension that a source file should use.
  static std::string getExtension(const Program &p) {
    if (p.getLangOpts().isCxx())
      return "cpp";
    return "c";
  }
};

#endif // DRIVERUTILS_H
