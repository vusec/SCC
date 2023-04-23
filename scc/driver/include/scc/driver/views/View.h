#ifndef VIEW_H
#define VIEW_H

#include "scc/driver/DriverState.h"

/// Base class for a screen in the interactive fuzzer UI.
struct View {
  virtual ~View() = default;

  virtual void draw(const DriverState &state) = 0;

  struct Input {
    struct Key {
      bool escaped = false;
      char c = 0;
      bool isArrowUp() const { return c == 'A' && escaped; }
      bool isArrowDown() const { return c == 'B' && escaped; }
    };

    std::vector<Key> keys;
  };
  virtual void handleInput(const Input &inputBuf, DriverState &state) = 0;
  virtual std::string getName() const = 0;
};

#endif
