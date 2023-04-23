#ifndef STRATEGYINSTANCE_H
#define STRATEGYINSTANCE_H

#include "Rng.h"

/// This encapsulates a mutation strategy with a history of taken decisions.
template <typename Strategy> class StrategyInstance {
  Rng rng;
  /// The strategy to use.
  Strategy strategy;
  typedef typename Strategy::Frag Frag;

  /// List of decisions that have been made so far.
  ///
  /// In the order in which they were made.
  std::vector<Frag> takenDecisions;

  typedef unsigned Weight;
  static constexpr unsigned weightPrecision = 10000;
  Weight getFragPickWeight(Frag f) {
    return static_cast<Weight>(strategy.get(f) * weightPrecision);
  }

public:
  StrategyInstance(size_t seed, const Strategy &s) : rng(seed), strategy(s) {}

  /// Perform a decision whether the given action should be done.
  bool decision(typename Strategy::Frag f) {
    // Do the actual decision.
    bool res = rng.withSuccessChance(strategy.get(f));
    // Record the result.
    if (res)
      takenDecisions.push_back(f);
    return res;
  }

  /// Returns true if the given list of options has any option with a
  /// non-zero chance to happen.
  bool hasViableOption(const std::vector<Frag> options) const {
    if (options.empty())
      return false;
    const float minimumViableChance = 0.001;
    for (Frag f : options) {
      if (strategy.get(f) > minimumViableChance)
        return true;
    }
    return false;
  }

  /// Pick a decision from the list based on its weight.
  Frag pick(const std::vector<Frag> l) {
    SCCAssert(!l.empty(), "No options provided?");

    typedef std::pair<Weight, Frag> WeightedFrag;
    std::vector<WeightedFrag> options;
    // Calculate the total weight of all options.
    Weight totalWeight = 0;
    for (Frag f : l) {
      const Weight weight = getFragPickWeight(f);
      if (weight == 0)
        continue;
      totalWeight += weight;
      options.push_back({weight, f});
    }
    // No decision has a chance to happen, so just pick a random one as a
    // fallback.
    if (totalWeight == 0)
      return rng.pickOneVec(l);

    // Pick a random one given the weights.
    Weight pickWeight = rng.getBelow(totalWeight);
    for (WeightedFrag f : options) {
      if (f.first >= pickWeight) {
        takenDecisions.push_back(f.second);
        return f.second;
      }
      pickWeight -= f.first;
    }
    SCCError("Incorrect weight calculation?");
  }

  /// Returns the list of decisions that have been taken so far.
  auto getTakenDecisions() const { return takenDecisions; }

  /// Forgets all taken decisions.
  void resetTakenDecisions() { takenDecisions.clear(); }
};

#endif // STRATEGYINSTANCE_H
