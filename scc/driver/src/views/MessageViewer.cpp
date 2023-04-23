#include "scc/driver/views/MessageViewer.h"

#include "scc/driver/DrawTools.h"
#include "scc/driver/DriverUtils.h"

#include <iostream>

void MessageViewer::draw(const DriverState &state) {
  if (state.getMessages().empty()) {
    DrawTools::printHeader("No messages", "█", "█");
    return;
  }

  SCCAssert(currentMsgIndex >= 0, "Negative message index?");

  const std::string header = " Message " + std::to_string(currentMsgIndex) +
                             " of " +
                             std::to_string(state.getMessages().size()) + " ";

  DrawTools::printHeader(header, "█", "█");
  std::cout << current.getContent() << "\n";
}

void MessageViewer::handleInput(const Input &input, DriverState &state) {
  for (const Input::Key key : input.keys) {
    if (key.isArrowUp())
      --currentMsgIndex;
    if (key.isArrowDown())
      ++currentMsgIndex;

    while (currentMsgIndex < 0)
      currentMsgIndex += state.getMessages().size();
    while (currentMsgIndex >= static_cast<long>(state.getMessages().size()))
      currentMsgIndex -= state.getMessages().size();

    current = state.getMessages().at(currentMsgIndex);
  }
}
