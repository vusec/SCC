#include "scc/program/LangOpts.h"

#include "scc/utils/SCCAssert.h"
#include "toml++/toml.h"

#include <fstream>
#include <iostream>

template <typename T> static bool setValue(T &, const toml::node &value);

template <> bool setValue(bool &b, const toml::node &value) {
  if (!value.is_boolean())
    return false;
  b = value.as_boolean()->get();
  return true;
}

template <> bool setValue(std::string &s, const toml::node &value) {
  if (!value.is_string())
    return false;
  s = value.as_string()->get();
  return true;
}

static std::string getAllLangOpts() {
  std::string res;
#define LANG_OPT(type, id, str, def)                                           \
  res += " * ";                                                                \
  res += str;                                                                  \
  res += "\n";
#include "scc/program/LangOpts.def"
  SCCAssert(!res.empty(), "No lang options declared?");
  res.resize(res.size() - 2U);
  return res;
}

OptError LangOpts::loadFromFile(std::string path) {
  toml::table j = toml::parse_file(path);
  try {
    j = toml::parse_file(path);
  } catch (const std::exception &e) {
    return Err(e.what());
  }

  toml::table *table = j["general"].as_table();
  if (!table)
    return Err("Missing [general] table");

  for (auto value : *table) {
    const std::string key(value.first.str());
#define LANG_OPT(type, id, str, def)                                           \
  if (key == str) {                                                            \
    if (!setValue(id, value.second)) {                                         \
      return Err("Invalid value for option '" + key + "'.");                   \
    }                                                                          \
    continue;                                                                  \
  }
#include "scc/program/LangOpts.def"

    return Err("Unknown language option '" + std::string(value.first.str()) +
               "'.\nOptions are:\n" + getAllLangOpts());
  }

  const std::map<std::string, Standard> standards = {
      {"c89", Standard::C89},     {"c99", Standard::C99},
      {"c++03", Standard::Cxx03}, {"c++11", Standard::Cxx11},
      {"c++17", Standard::Cxx17},
  };

  auto iter = standards.find(standard);
  if (iter == standards.end()) {
    std::string err = "Unknown standard string '" + standard + "'.\n";
    err += "Valid standard strings are:\n";
    for (auto &p : standards)
      err += " * '" + p.first + "'\n";
    return Err(err);
  }
  standardEnum = iter->second;

  return {};
}
