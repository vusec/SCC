#ifndef EXECUTOR_H
#define EXECUTOR_H

#include <optional>
#include <string>

/// Utils for test oracle operations (parsing output, running it).
class Executor {
public:
  static std::optional<std::string> getValue(std::string output,
                                             std::string key);

  /// Whether the given output has a fuzzer message with the given key.
  /// e.g. 'FUZZ:MSG:'
  static bool hasValue(std::string output, std::string key);

  /// Runs the given shell command and returns its stdout output.
  static std::string exec(std::string cmd);
};

#endif // EXECUTOR_H
