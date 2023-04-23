#ifndef DRIVER_STATE_H
#define DRIVER_STATE_H

#include "scc/mutator-utils/Scheduler.h"
#include "scc/program/Program.h"

/// The fuzzer state that the different views see.
struct DriverState {
  DriverState(SchedulerBase &scheduler, std::string evalCommand,
              std::string saveDir);

  SchedulerBase &scheduler;
  Program lastProg;

  /// The command that should be run on each program.
  std::string evalCommand;
  /// The directory path to save interesting cases to.
  std::string saveDir;

  /// How many programs we put into the output folder so far.
  size_t savedCases = 0;
  /// Runtime in milliseconds.
  size_t millisTotal = 1;
  /// How many iterations we did.
  size_t iteration = 0;

  /// A unique string that should be used in the file name when saving programs.
  std::string uniqueFileID;

  /// How much time we spent in the oracle so far.
  size_t millisExe = 0;
  /// How many programs we actually executed (doesn't count cache hits).
  size_t execs = 0;

  typedef std::function<std::string(const Program &p)> Annotation;
  /// Function that can prepend some code to the final program.
  Annotation prefixFunc;
  /// Function that can append some code to the final program.
  Annotation suffixFunc;

  /// True if we're in manual stepping mode where the user presses space to
  /// go from one iteration to the next.
  bool manualStepping = false;
  /// How many manual steps are queued up in the input.
  unsigned manualSteps = 0;

  /// Calculates the expected executions per second.
  double getExecsPerSec() const {
    return 1000.0 * execs / static_cast<int>(millisTotal);
  }

  /// A message from the oracle.
  struct Message {
    std::string timestamp;
    std::string content;

    std::string getContent() const { return content; }
    std::string getTimeStamp() const { return timestamp; }
  };

  /// Try to print the given program to the given output path.
  OptError printProg(const Program &p, std::string outPath);

  /// Save a program to the output folder.
  void saveProg(const Program &p, std::string prefix);

  /// Add a message from the oracle to the internal storage.
  void addMessageWithTimestamp(std::string msg, std::string timestamp) {
    messages.push_back({timestamp, msg});
    while (messages.size() > maxStoredMessages)
      messages.pop_front();
  }

  /// Returns the list of all messages received so far.
  std::deque<Message> getMessages() const { return messages; }

private:
  /// Maximum number of messages that we should store before deleting old ones.
  std::size_t maxStoredMessages = 1000;
  /// List of stores messages from the oracle.
  std::deque<Message> messages;
};

#endif
