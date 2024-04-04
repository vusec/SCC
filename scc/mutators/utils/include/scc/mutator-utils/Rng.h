#pragma once

#include "scc/utils/SCCAssert.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <list>
#include <random>

struct EntrophyVec {
  typedef unsigned char result_type;

  explicit EntrophyVec(std::string_view input) {
    for (char c : input)
      entrophy.push_back(static_cast<result_type>(c));
  }

  result_type pop() {
    result_type result = entrophy.back();
    entrophy.pop_back();
    return result;
  }
  bool hasData() const { return !entrophy.empty(); }

private:
  std::vector<result_type> entrophy;
};

struct RngSource {
  typedef uint64_t Seed;
  explicit RngSource(Seed seed) : seed(seed), gen(seed) {}

  explicit RngSource(EntrophyVec &entrophy) : entrophy(&entrophy) {}

  typedef EntrophyVec::result_type result_type;
  static constexpr result_type min() { return 0; }
  static constexpr result_type max() {
    return std::numeric_limits<result_type>::max();
  }
  result_type operator()() {
    if (entrophy && entrophy->hasData())
      return entrophy->pop();
    return gen();
  }

  RngSource spawnChild() {
    (void)gen();
    return *this;
  }

private:
  EntrophyVec *entrophy = nullptr;
  Seed seed = 0;
  std::ranlux48_base gen;
};

/// Random number generation utility class.
class Rng {
  /// The base RNG used for generating everything else.
  RngSource gen;

public:
  explicit Rng(RngSource src) : gen(src) {}

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

  Rng spawnChild() {
    Rng result(gen.spawnChild());
    return result;
  }

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
