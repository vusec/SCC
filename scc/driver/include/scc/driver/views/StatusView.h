#ifndef STATUS_VIEW_H
#define STATUS_VIEW_H

#include "scc/driver/views/View.h"

/// The main view of the program generator.
///
/// Shows a quick overview of the current program and some basic stats.
struct StatusView : View {
  ~StatusView() override = default;

  void draw(const DriverState &state) override;
  void handleInput(const Input &input, DriverState &state) override;
  std::string getName() const override { return "Status"; }

private:
  bool printLast = true;
  unsigned startLine = 0;
};

#endif
