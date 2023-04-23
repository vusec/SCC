#include "scc/driver/Executor.h"

#include <array>
#include <memory>
#include <stdexcept>

std::optional<std::string> Executor::getValue(std::string output,
                                              std::string key) {
  output = "\n" + output;
  key = "\n" + key;
  size_t pos = output.find(key);
  if (pos == std::string::npos)
    return {};
  std::string rest = output.substr(pos + key.size());
  size_t endLine = rest.find("\n");
  if (endLine == std::string::npos)
    return rest;
  if (endLine == 0)
    return "";
  return rest.substr(0, endLine);
}

bool Executor::hasValue(std::string output, std::string key) {
  output = "\n" + output + "\n";
  key = "\n" + key + "\n";
  size_t pos = output.find(key);
  if (pos == std::string::npos)
    return false;
  return true;
}

std::string Executor::exec(std::string cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"),
                                                pclose);
  if (!pipe)
    throw std::runtime_error("popen() failed!");
  while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    result += buffer.data();
  return result;
}
