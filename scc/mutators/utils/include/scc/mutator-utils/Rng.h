#pragma once

#include "scc/utils/SCCAssert.h"
#include <algorithm>
#include <cassert>
#include <list>
#include <random>

/// Random number generation utility class.
class Rng {
  /// The base RNG used for generating everything else.
  std::ranlux48_base gen;

public:
  explicit Rng(uint64_t seed) : gen(seed) {}

  /// Returns a random float between 0 and 1.
  float get0To1() {
    std::uniform_real_distribution<float> dist(0, 1);
    return dist(gen);
  }

  /// Returns a random float between -1 and 1.
  float getMin1To1() {
    std::uniform_real_distribution<float> dist(-1, 1);
    return dist(gen);
  }

  /// Return a new seed for another Rng instance.
  uint64_t makeSeed() { return gen(); }

  /// Returns a number below or equal the given max value.
  template <typename T> T getBelow(T max) {
    if (max == 0)
      return 0;
    std::uniform_int_distribution<T> dist(0, max);
    return dist(gen);
  }

  /// Make a decision that passes with the given success chance. Returns true
  /// if the decision turned out positive.
  bool withSuccessChance(double chance) {
    std::uniform_real_distribution<double> dist(0, 1);
    auto r = dist(gen);
    return r < chance;
  }

  bool flipCoin() { return withSuccessChance(0.5); }

  /// Pick a random index for a container of the given size.
  size_t pickIndex(size_t size) {
    assert(size != 0);
    std::uniform_int_distribution<size_t> dist(0, size - 1U);
    return dist(gen);
  }

  /// Return a random element from the given initializer list.
  template <typename T> T pickOne(std::initializer_list<T> values) {
    SCCAssert(values.size() != 0, "Empty list?");
    return *(values.begin() + pickIndex(values.size()));
  }

  /// Return a random element from the given initializer list.
  template <typename T> T pickOne(std::list<T> values) {
    SCCAssert(values.size() != 0, "Empty list?");
    return *(values.begin() + pickIndex(values.size()));
  }

  /// Return a random element from the given string.
  char pickOneStr(const std::string &s) {
    SCCAssert(!s.empty(), "Empty string?");
    return s.at(pickIndex(s.size()));
  }
  template <typename T> T &pickOneVec(std::vector<T> &values) {
    SCCAssert(!values.empty(), "Empty vector?");
    size_t index = pickIndex(values.size());
    return *(values.begin() + index);
  }

  /// Return a random element from the given vector.
  template <typename T> const T &pickOneVec(const std::vector<T> &values) {
    SCCAssert(!values.empty(), "Empty vector?");
    size_t index = pickIndex(values.size());
    return *(values.begin() + index);
  }

  /// Shuffles the given list in situ.
  template <class T> void shuffle(T &list) {
    std::shuffle(list.begin(), list.end(), gen);
  }

  /// Runs the given vector of lambas in a random order.
  ///
  /// When the first function in the list returns a Maybe containing no error,
  /// this function returns the value.
  ///
  /// Otherwise a default constructed error is returned.
  template <class T>
  auto runRandomly(const std::vector<T> x) -> decltype(T()()) {
    typedef decltype(T()()) ReturnT;

    std::vector<std::size_t> toVisit;
    for (const auto &_ : x)
      toVisit.push_back(toVisit.size());
    shuffle(toVisit);

    for (const std::size_t i : toVisit) {
      auto res = x.at(i)();
      if (res.isErr())
        continue;
      return res;
    }
    return ReturnT();
  }
};
