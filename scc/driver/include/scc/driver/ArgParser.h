#ifndef ARGPARSER_H
#define ARGPARSER_H

#include <limits>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

/// Parses command line arguments.
struct ArgParser {
  typedef std::optional<std::string> Error;

private:
  std::vector<std::string> args;
  bool consume(std::string &arg, const std::string &prefix);

  std::unordered_map<std::string, std::string> aliases = {
      {"--no-reduce", "--reducer-tries=0"},
      {"--stop-after-hit", "--stop-after-hits=1"}};

  Error parseSingleArg(std::string arg);

public:
  Error parse(int argc, char **argv);

  std::string getEvalCommand() const {
    std::string cmd;
    for (const std::string &a : args)
      cmd += " \"" + a + "\"";
    return cmd;
  }

  std::string argv0;

  std::string generator = "unsafe";

  std::string optsFile;
  std::string saveDir = "saved_testcases";
  size_t tries = 3000;
  size_t queueSize = 300;
  unsigned mutatorScale = 1;
  size_t uiUpdateMs = 200;
  bool simpleUI = false;
  bool splash = false;
  bool wrapMain = true;
  bool manualStepping = false;
  size_t stopAfter = std::numeric_limits<size_t>::max();
  // Stop after 100k test cases are saved. Avoids filling up disk space when
  // some basic setup is messed up and causes FPs.
  std::size_t stopAfterHits = 100000;
  unsigned reducerTries = 3000;
  size_t seed = 0;

  std::vector<std::string> unknownArgs;
};

#endif // ARGPARSER_H
