#ifndef COLOROUTPUT_H
#define COLOROUTPUT_H

#include <string>

/// Simple console color output util.
class ColorOutput {
  // TODO: merge with OutStream?
public:
  enum Color {
    Black,
    Red,
    Green,
    yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BoldBlack,
    BoldRed,
    BoldGreen,
    BoldYellow,
    BoldBlue,
    BoldMagenta,
    BoldCyan,
    BoldWhite
  };

  static void print(Color c, std::string s);
  static void print(Color c, const char *s) { print(c, std::string(s)); }
  template <typename T> static void print(Color c, T t) {
    print(c, std::to_string(t));
  }

private:
  static void startColor(Color c);
};

#endif // COLOROUTPUT_H
