#include "scc/driver/DriverState.h"

#include "scc/driver/DriverUtils.h"

#include <filesystem>

DriverState::DriverState(SchedulerBase &scheduler, std::string evalCommand,
                         std::string saveDir)
    : scheduler(scheduler), evalCommand(evalCommand), saveDir(saveDir) {
  std::time_t t = std::time(nullptr);
  std::random_device rd;

  // Generate a short id for the fuzzer.
  auto hash = std::hash<std::string>()("P:" + std::to_string(getpid()) +
                                       "_S:" + std::to_string(t) +
                                       "_R:" + std::to_string(rd()));

  std::stringstream sstream;
  sstream << std::hex << hash;
  uniqueFileID = sstream.str();
  uniqueFileID.resize(8);
}

OptError DriverState::printProg(const Program &p, std::string outPath) {
  std::string absPath = std::filesystem::absolute(outPath).string();
  DriverUtils::FileOut out(outPath);
  out << "// Run: " << evalCommand << " " << absPath << "\n";
  out << prefixFunc(p);
  if (OptError err = p.print(out))
    return err;
  out << suffixFunc(p);
  return {};
}

void DriverState::saveProg(const Program &p, std::string prefix) {
  while (true) {
    ++savedCases;
    std::string fileName = saveDir + "/" + prefix;
    fileName += std::to_string(savedCases) + ".";
    fileName += uniqueFileID + ".";
    fileName += DriverUtils::getExtension(p);
    if (std::filesystem::exists(fileName))
      continue;
    printProg(p, fileName);
    break;
  }
}
