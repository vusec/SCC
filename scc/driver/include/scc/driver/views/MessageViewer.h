#ifndef MESSAGE_VIEWER_H
#define MESSAGE_VIEWER_H

#include "scc/driver/views/View.h"

#include <deque>
#include <string>

/// Allows looking at oracle messages in more detail.
struct MessageViewer : View {
  ~MessageViewer() override = default;
  void draw(const DriverState &state) override;
  void handleInput(const Input &input, DriverState &state) override;
  std::string getName() const override { return "Messages"; }

private:
  long currentMsgIndex = 0;
  DriverState::Message current;
};

#endif // MESSAGE_VIEWER_H
