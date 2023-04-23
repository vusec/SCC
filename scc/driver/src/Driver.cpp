#include "scc/driver/Driver.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#include <utility>

#include "scc/driver/views/MessageViewer.h"
#include "scc/driver/views/StatusView.h"

#include "scc/driver/ColorOutput.h"
#include "scc/driver/DrawTools.h"
#include "scc/driver/DriverUtils.h"
#include "scc/driver/Executor.h"
#include "scc/driver/FancyProgramPrinter.h"
#include "scc/driver/PretentiousUI.h"
#include "scc/mutator-utils/Scheduler.h"

static Driver *globalDriver = nullptr;

static bool printLast = true;

static SchedulerBase::Feedback evalProg(const Program &p) {
  auto &state = globalDriver->getState();
  if (printLast)
    state.lastProg = p;

  SchedulerBase::Feedback result;

  std::string outPath = "/tmp/gen_source" + std::to_string(getpid()) + "." +
                        DriverUtils::getExtension(p);

  // Delete the file at the end.
  DriverUtils::FileCleanup cleanup(outPath);

  state.printProg(p, outPath);
  const std::string scoreNeedle = "FUZZ:SCORE:";
  std::string feedbackStr;
  size_t exeTime = 0;
  {
    DriverUtils::Timer timer(exeTime);
    feedbackStr = Executor::exec(state.evalCommand + " " + outPath + " 2>&1");
  }
  state.execs += 1;
  state.millisExe += exeTime;

  auto now = std::chrono::system_clock::now();
  const std::time_t nowT = std::chrono::system_clock::to_time_t(now);
  std::stringstream timeS;
  timeS << std::put_time(std::localtime(&nowT), "%T");
  const std::string timeStr = timeS.str();

  auto addMsg = [&timeStr](std::string msg) {
    globalDriver->getState().addMessageWithTimestamp(msg, timeStr);
  };

  result.interesting = Executor::hasValue(feedbackStr, "FUZZ:HIT");
  result.deadEnd = Executor::hasValue(feedbackStr, "FUZZ:DEAD");
  if (auto msg = Executor::getValue(feedbackStr, "FUZZ:MSG:")) {
    result.msg = *msg;
    addMsg(*msg);
  }

  std::optional<std::string> scoreStr =
      Executor::getValue(feedbackStr, scoreNeedle);
  if (!scoreStr) {
    result.interesting = true;
    addMsg("No score from oracle? Output: " + feedbackStr);
    return result;
  }
  try {
    result.score = std::stol(scoreStr->c_str());
  } catch (std::invalid_argument e) {
    result.interesting = true;
    addMsg("Oracle score not an int: " + *scoreStr);
    return result;
  } catch (std::out_of_range e) {
    result.interesting = true;
    addMsg("Oracle score out of range: " + *scoreStr);
    return result;
  }
  return result;
}

void Driver::updateUIIfNecessary() {
  auto now = std::chrono::high_resolution_clock::now();
  size_t deltaTime =
      std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUIUpdate)
          .count();
  if (deltaTime > minUIUpdateInterval) {
    lastUIUpdate = now;
    if (simpleUI || !DriverUtils::hasViableWinSize())
      doSimpleUI();
    else {
      DriverUtils::clearScreen();
      std::vector<std::string> headers;
      for (const auto &view : views)
        headers.push_back(view->getName());

      DrawTools::drawTopBar(headers, currentView);

      views.at(currentView)->draw(state);
    }
  }
}

void Driver::doSimpleUI() {
  auto &scheduler = state.scheduler;
  std::cout << "Iteration: " << std::setw(8) << state.iteration << " ";
  std::cout << " | Score: " << std::setw(3) << scheduler.getBestScore();
  std::cout << " | Found interesting cases: " << std::setw(3)
            << scheduler.getNumFindings();
  std::cout << " | Execs/s " << std::setw(4) << std::fixed
            << std::setprecision(2) << state.getExecsPerSec();
  std::cout << std::setw(0);
  std::cout << " | Desperation: " << std::setw(3) << scheduler.getDesperation();
  std::cout << " | Mutation info: " << std::setw(20)
            << scheduler.getLastStratInfo();
  std::cout << "\n";
  std::cout.flush();
}

static bool endsWith(std::string s, std::string suffix) {
  if (s.size() < suffix.size())
    return false;
  return s.substr(s.size() - suffix.size(), suffix.size()) == suffix;
}

void Driver::handleInput() {
  std::array<char, 5> buf;
  buf.fill(0);
  fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL) | O_NONBLOCK);

  int c = 0;
  View::Input input;
  std::string backlog;
  while ((c = getc(stdin)) != EOF) {
    const char maxViewNum = '1' + views.size();
    if (c >= '1' && c < maxViewNum) {
      currentView = c - '1';
      continue;
    }
    if (c == 'q')
      exitRequested = true;
    View::Input::Key key;
    key.c = c;
    key.escaped = endsWith(backlog, "\x1B[");
    backlog.push_back(c);
    input.keys.push_back(key);
  }
  views.at(currentView)->handleInput(input, state);
}

Driver::Driver(SchedulerBase &scheduler, std::string evalCommand,
               StepFunc stepFunc, std::string saveDir)
    : state(scheduler, evalCommand, saveDir), stepFunc(stepFunc) {
  globalDriver = this;

  views.emplace_back(std::make_unique<StatusView>());
  views.emplace_back(std::make_unique<MessageViewer>());

  lastUIUpdate = std::chrono::high_resolution_clock::now();
}

void Driver::run(bool splash) {
  // Disable character echoing.
  static struct termios term_flags;
  tcgetattr(STDIN_FILENO, &term_flags);
  term_flags.c_lflag &= ~(ICANON);
  term_flags.c_lflag &= ~(ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &term_flags);

  setbuf(stdout, nullptr);

  if (splash)
    PretentiousUI::render();

  state.scheduler.setEvalFunction([](const Program &p) { return evalProg(p); });
  // Display the initial program on the UI.
  state.lastProg = state.scheduler.getBestProg();

  std::chrono::steady_clock::time_point begin =
      std::chrono::steady_clock::now();

  while (!state.scheduler.finished() && !exitRequested) {

    if (canStep()) {
      ++state.iteration;
      DriverUtils::Timer timer(state.millisTotal);
      stepFunc();
    }

    auto toSave = state.scheduler.popInteresting();
    for (const Program &p : toSave)
      state.saveProg(p, "id_");

    handleInput();
    updateUIIfNecessary();
  }
  if (!simpleUI)
    DriverUtils::clearScreen();

  std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
  std::cout << "Finished fuzzing!\n";
  std::cout << "Found bugs: " << state.savedCases << "\n";
  std::cout << "Total iterations: " << state.iteration << "\n";
  std::cout << "Total executions: " << state.execs << "\n";
  unsigned elapsedMs =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - begin)
          .count();
  std::cout << "Total time (ms) : " << elapsedMs << "\n";
}
