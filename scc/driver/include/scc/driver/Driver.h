#ifndef DRIVER_H
#define DRIVER_H

#include "scc/driver/DriverState.h"
#include "scc/driver/views/MessageViewer.h"
#include "scc/mutator-utils/Scheduler.h"

#include <chrono>

/// Provides glue that connects a scheduler to the evaluation function and
/// renders the UI.
class Driver {
public:
  typedef std::function<void()> StepFunc;

private:
  DriverState state;

  StepFunc stepFunc;

  size_t minUIUpdateInterval = 30;
  bool simpleUI = false;

  std::chrono::high_resolution_clock::time_point lastUIUpdate;

  bool canStep() {
    if (!state.manualStepping)
      return true;
    if (state.manualSteps > 0) {
      state.manualSteps -= 1;
      return true;
    }
    return false;
  }

  void updateUIIfNecessary();
  void doUI();
  void doSimpleUI();
  void handleInput();

  std::vector<std::unique_ptr<View>> views;
  std::size_t currentView = 0;

  bool exitRequested = false;
public:
  Driver(SchedulerBase &scheduler, std::string evalCommand, StepFunc stepFunc,
         std::string saveDir);

  void setUpdateInterval(size_t millis) { minUIUpdateInterval = millis; }
  void setSimpleUI(bool b) { simpleUI = b; }
  void setManualStepping(bool b) { state.manualStepping = b; }

  typedef DriverState::Annotation Annotation;
  void setPrefixFunc(Annotation a) { state.prefixFunc = a; }
  void setSuffixFunc(Annotation a) { state.suffixFunc = a; }

  void run(bool splash = false);

  DriverState &getState() { return state; }
};

#endif // DRIVER_H
