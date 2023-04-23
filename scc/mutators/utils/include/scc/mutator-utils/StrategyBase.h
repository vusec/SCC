#ifndef STRATEGYBASE_H
#define STRATEGYBASE_H

#include "Rng.h"
#include <algorithm>
#include <vector>

/// Contains methods for creating a list of weighted decisions.
template <typename Derived, typename DerivedFrag> class StrategyBase {
public:
  /// Assign every decision a random chance.
  void randomize(Rng &rng) {
    for (float &v : values)
      if (rng.withSuccessChance(0.1f))
        v += legalizeValue((rng.getMin1To1() * 0.4f) + 0.5f);
    name = "Random strategy " + std::to_string(generation);
    ++generation;
    if (rng.withSuccessChance(0.7f))
      scale += (int)rng.getBelow(6) - 1;
    scale = std::clamp<unsigned>(scale, 1, 10);
  }

  unsigned generation = 1;
  unsigned scale = 1;

  /// Returns the 0-1 chance that the given decision should
  /// be taken.
  float get(DerivedFrag f) const { return values[getIndex(f)]; }

  void set(DerivedFrag f, float v) { values[getIndex(f)] = legalizeValue(v); }

  /// Set all chances to zero.
  void zeroFill() {
    size_t oldSize = values.size();
    values.clear();
    values.resize(oldSize, 0.0f);
  }

  /// Return the user-readable name of this strategy.
  std::string_view getName() const { return name; }

  std::vector<float> &getValueVecRef() { return values; }

  void setName(std::string n) { name = n; }

protected:
  float legalizeValue(float v) { return std::max(0.0f, std::min(1.0f, v)); }
  std::size_t getIndex(DerivedFrag f) const {
    return static_cast<std::size_t>(f);
  }
  std::vector<float> values;
  std::string name = "default";
};

#endif // STRATEGYBASE_H
