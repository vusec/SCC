#include "scc/driver/ArgParser.h"

bool ArgParser::consume(std::string &arg, const std::string &prefix) {
  if (arg.find(prefix) == 0) {
    arg = arg.substr(prefix.size());
    return true;
  }
  return false;
}

ArgParser::Error ArgParser::parseSingleArg(std::string arg) {
  if (aliases.count(arg))
    arg = aliases[arg];

  if (consume(arg, "--tries=")) {
    tries = std::stoul(arg);
    if (tries == 0)
      return "Invalid or 0 passed to --tries";
    return {};
  } else if (consume(arg, "--queue-size=")) {
    queueSize = std::stoul(arg);
    if (queueSize == 0)
      return "Invalid or 0 passed to --queueSize";
    return {};
  } else if (consume(arg, "--seed=")) {
    seed = std::stoul(arg);
    if (seed == 0)
      return "Invalid or 0 passed to --seed";
    return {};
  } else if (consume(arg, "--generator=")) {
    generator = arg;
    if (generator.empty())
      return "Have to specify a generator to --generator=";
    return {};
  } else if (consume(arg, "--reducer-tries=")) {
    reducerTries = std::stoul(arg);
    return {};
  } else if (consume(arg, "--lang-opts=")) {
    optsFile = arg;
    if (optsFile.empty())
      return "Have to specify a path to --lang-opts=";
    return {};
  } else if (consume(arg, "--output=")) {
    saveDir = arg;
    if (saveDir.empty())
      return "Have to specify a path to --output=";
    return {};
  } else if (consume(arg, "--scale=")) {
    mutatorScale = std::stoul(arg);
    if (mutatorScale == 0)
      return "Invalid or 0 passed to --scale";
    return {};
  } else if (consume(arg, "--ui-update=")) {
    uiUpdateMs = std::stoul(arg);
    if (mutatorScale == 0)
      return "Invalid or 0 passed to --ui-update=";
    return {};
  } else if (consume(arg, "--stop-after=")) {
    stopAfter = std::stoul(arg);
    if (stopAfter == 0)
      return "Invalid or 0 passed to --stop-after=";
    return {};
  } else if (consume(arg, "--stop-after-hits=")) {
    stopAfterHits = std::stoul(arg);
    if (stopAfterHits == 0)
      return "Invalid or 0 passed to --stop-after-hits=";
    return {};
  } else if (arg == "--no-wrap") {
    wrapMain = false;
    return {};
  } else if (arg == "--step") {
    manualStepping = true;
    return {};
  } else if (arg == "--simple-ui") {
    simpleUI = true;
    return {};
  } else if (arg == "--splash") {
    splash = true;
    return {};
  }
  unknownArgs.push_back(arg);
  return {};
}

ArgParser::Error ArgParser::parse(int argc, char **argv) {
  argv0 = *argv;
  bool inOracleArgs = false;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    if (arg == "--") {
      if (!inOracleArgs) {
        inOracleArgs = true;
        continue;
      }
    }
    if (inOracleArgs) {
      args.push_back(arg);
      continue;
    }
    if (auto err = parseSingleArg(arg))
      return err;
  }
  if (args.empty())
    return "No oracle args. Please append them behind a '--'.";
  return {};
}
