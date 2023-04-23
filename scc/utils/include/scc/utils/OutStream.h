#pragma once

#include "scc/utils/SCCAssert.h"
#include <cassert>
#include <string>
#include <string_view>

/// Output stream to a single source file.
class OutStream {

public:
  enum Color {
    Default,
    Black,
    White,
    Red,
    Magenta,
    Blue,
    Green,
    Cyan,
    Yellow,
    BoldBlack,
    BoldWhite,
    BoldRed,
    BoldMagenta,
    BoldBlue,
    BoldGreen,
    BoldCyan,
    BoldYellow
  };

private:
  /// The current color that should be used when printing.
  Color color = Color::Default;

protected:
  /// The current indentation level in characters.
  unsigned indent = 0;
  /// True if this is 'informal' output only read by humans.
  /// This means we can skip some boilerplate to make code more readable.
  bool isInformal = false;
  /// True if the output supports ASCII color escapes.
  bool supportsColor = false;
  /// True if we're currently printing 'hidden' characters such as colors
  /// and other control sequences. This is set by OutStream itself.
  bool printingHidden = false;
  bool isPrintingHiddenChars() { return printingHidden; }
  virtual void writeImpl(std::string_view s) = 0;

public:
  virtual ~OutStream();

  void write(std::string_view s) { writeImpl(s); }

  OutStream &printColor(Color c, std::string_view s) {
    Color origColor = color;
    setColor(c);
    write(s);
    setColor(origColor);
    return *this;
  }

  void setColor(Color c);

  void increaseIndent() { indent += 2; }
  void decreaseIndent() {
    SCCAssert(indent != 0, "Trying to decrease 0 indent?");
    indent -= 2;
  }

  void printIndent() { writeImpl(std::string(indent, ' ')); }

  /// True if this is output goes to something that won't be parsed (e.g., the
  /// preview the user sees).
  /// This means we can print weird markers and stuff that isn't valid in
  /// the target language.
  bool isInformalOutput() const { return isInformal; }
};

inline OutStream &operator<<(OutStream &s, std::string_view v) {
  s.write(v);
  return s;
}

inline OutStream &operator<<(OutStream &s, unsigned v) {
  s.write(std::to_string(v));
  return s;
}

/// RAII for bumping indentation and decreasing it on scope exit.
struct IndentScope {
  OutStream &out;
  IndentScope(OutStream &o) : out(o) { o.increaseIndent(); }
  ~IndentScope() { out.decreaseIndent(); }
};

/// An OutStream that appends to a string.
class OutString : public OutStream {
  std::string storage;

public:
  OutString(bool fancy = false) {
    isInformal = fancy;
    supportsColor = fancy;
  }
  virtual ~OutString();
  virtual void writeImpl(std::string_view s) { storage.append(std::string(s)); }

  const std::string &getStr() const { return storage; }
};

/// An OutStream that writes to stderr.
struct StdErrOutStream : public OutStream {
  StdErrOutStream() {
    isInformal = true;
    supportsColor = true;
  }
  virtual ~StdErrOutStream();
  virtual void writeImpl(std::string_view s);
};

/// An OutStream that just hashes it's output.
class HashStream : public OutStream {
public:
  virtual ~HashStream();
  virtual void writeImpl(std::string_view s) {
    hash ^= std::hash<std::string_view>()(s);
  }

  typedef size_t Hash;
  size_t getHash() const { return hash; }

private:
  size_t hash = 0;
};
