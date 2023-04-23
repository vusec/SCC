#include "scc/driver/views/StatusView.h"

#include "scc/driver/DrawTools.h"
#include "scc/driver/DriverUtils.h"
#include "scc/driver/FancyProgramPrinter.h"

#include <iomanip>
#include <sstream>

void StatusView::draw(const DriverState &state) {
  SchedulerBase &scheduler = state.scheduler;

  size_t percentage = static_cast<unsigned>(
      state.millisExe / static_cast<double>(state.millisTotal) * 100);

  std::stringstream execs;
  execs << std::fixed << std::setprecision(2) << state.getExecsPerSec();

  DrawTools::printLine(
      "Iterations: " + std::to_string(state.iteration) + " of which " +
      std::to_string(scheduler.getCacheHits()) + " (" +
      std::to_string(scheduler.getCacheHitRate()) + "%) hit cache. Execs/s: " +
        execs.str());
  DrawTools::printLine("Found interesting programs: " +
                       std::to_string(scheduler.getNumFindings()));

  DrawTools::printLine(
      "Current mutation strategy info: " + scheduler.getLastStratInfo() +
      ", Desperation score: " + std::to_string(scheduler.getDesperation()));

  DrawTools::printLine("Oracle time: " + std::to_string(percentage) +
                       "% of runtime spent in oracle");

  auto score = scheduler.getBestScore();
  std::string scoreStr;
  if (score == std::numeric_limits<SchedulerBase::Score>::min()) {
    scoreStr = "Minimum score";
  } else
    scoreStr = std::to_string(scheduler.getBestScore());

  DrawTools::printLine("Oracle score for current top queue entry: " + scoreStr);

  DrawTools::printHeader(" Messages from oracle ", "┣", "┫");

  const unsigned maxMessages = 5;
  auto messages = state.getMessages();
  std::reverse(messages.begin(), messages.end());
  for (unsigned i = 0; i < maxMessages; ++i) {
    if (i < messages.size()) {
      const DriverState::Message message = messages.at(i);
      std::string msg = message.getTimeStamp() + ": " + message.getContent();
      msg.resize(std::min<std::size_t>(
          DriverUtils::getTerminalSize().ws_col - 10U, msg.size()));
      std::size_t firstNewline = msg.find_first_of('\n');
      if (firstNewline != std::string::npos)
        msg = msg.substr(0, firstNewline);
      DrawTools::printLine(msg);
    } else {
      DrawTools::printLine("");
    }
  }

  const Program &p = printLast ? state.lastProg : scheduler.getBestProg();

  if (printLast)
    DrawTools::printHeader(" Current test case ", "┗", "┛");
  else
    DrawTools::printHeader(" Best test case ", "┗", "┛");

  {
    auto window = DriverUtils::getTerminalSize();
    FancyProgramPrinter str(window.ws_row - 15, window.ws_col - 1, startLine);
    p.print(str).assumeSuccess("Failed to print program for UI");
  }
  std::cout.flush();
}

void StatusView::handleInput(const Input &input, DriverState &state) {

  for (const Input::Key key : input.keys) {
    if (key.isArrowUp() && startLine > 0)
      --startLine;
    if (key.isArrowDown())
      ++startLine;

    switch (key.c) {
    case 'v':
      printLast = !printLast;
      break;
    case 'c':
      state.scheduler.requestQueueReset();
      break;
    case 's': {
      state.saveProg(state.lastProg, "saved_");
      break;
    }

    case ' ':
      if (state.manualStepping)
        state.manualSteps += 1;
      break;
    }
  }
}
