#include "scc/program/IdentTable.h"
#include <unordered_set>

bool IdentTable::isValidName(std::string s) const {
  if (s.empty())
    return false;
  char first = s.front();
  if (first >= '0' && first <= '9')
    return false;

  const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                            "abcdefghijklmnopqrstuvwxyz"
                            "0123456789_";
  std::unordered_set<char> validChars;
  for (char c : chars)
    validChars.insert(c);

  for (char c : s)
    if (validChars.count(c) == 0)
      return false;
  return true;
}
