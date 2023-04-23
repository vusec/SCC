#ifndef REDUCER_H
#define REDUCER_H

#include "SchedulerBase.h"

/// A scheduler specifically aimed at reducing a given program.
///
/// This tries to find the smallest possible program that is still considered
/// 'interesting'.
template <typename GeneratorT> class Reducer {
  typedef typename GeneratorT::Strategy Strategy;
  typedef SchedulerBase::Feedback Feedback;
  typedef SchedulerBase::FeedbackFunc FeedbackFunc;
  typedef SchedulerBase::Score Score;

  GeneratorT gen;

  std::vector<Strategy> strategies;

  /// The program that is being reduced.
  Program toReduce;
  /// The rng to use.
  Rng rng;
  /// The feedback function that determines whether a reduced program is still
  /// considered interesting.
  FeedbackFunc feedback;
  /// The size of the orignal program.
  ///
  /// Count as characters in the string representation.
  size_t originalSize = 0;
  /// The size of the smallest program found so far.
  size_t lastSize = 0;

  /// Returns the size of the program.
  size_t getProgSize(const Program &p) {
    OutString out;
    p.print(out).assumeSuccess("Failed to print to calculate size");
    return out.getStr().size();
  }

  /// Maximum number of tries to mutate a program to find a smaller
  /// interesting version.
  unsigned maxTries = 3000;
  /// How many mutation tries are left do at the moment before we give up.
  unsigned triesLeft = maxTries;
  /// How many mutations to find a version that is smaller but maybe not
  /// interesting.
  unsigned mutateToReduceTries = 10;

  /// String that describes by how much we reduced the program so far.
  std::string reducedPercentage() const {
    return std::to_string((int)(100 * lastSize / (float)originalSize));
  }

  /// Cache of already seen reduced versions.
  ProgramCache cache;

public:
  Reducer(FeedbackFunc feedback, uint64_t seed, const Program &p)
      : toReduce(p), rng(seed), feedback(feedback) {
    originalSize = lastSize = getProgSize(p);
    strategies = Strategy::makeReductionStrategies();
  }

  /// Whether this reducer is done reducing.
  bool finished() const { return triesLeft == 0; }

  /// Performs N reduction steps.
  void steps(unsigned number) {
    for (unsigned i = 0; i < number; ++i)
      step();
  }

  /// Perform a single reduction step.
  ///
  /// @return A textual description of what the reduction step did.
  std::string step() {
    if (finished())
      return "Done reducing";
    triesLeft -= 1;
    const std::string res =
        "Reducing (Tries left: " + std::to_string(triesLeft) + ", " +
        "Reduced size " + reducedPercentage() + "%) ";

    Program p;
    size_t newSize = 0;
    for (unsigned i = 1; i <= mutateToReduceTries; ++i) {
      p = toReduce;
      const Strategy &strat = rng.pickOneVec(strategies);
      auto taken = gen.reduce(p, rng.makeSeed(), strat);

      // If the program is malformed, skip it.
      if (!p.canPrint())
        continue;

      // If we seen this before then retry.
      if (cache.isInCacheNoInsert(p))
        continue;

      // If the program is smaller than the last version then we reduced it.
      newSize = getProgSize(p);
      if (newSize < lastSize)
        break;

      if (i == mutateToReduceTries)
        return "Failed to smaller mutated program";
    }

    // Malformed program, ignore it.
    if (!p.canPrint())
      return res + " - Failed to find program variant";

    // We already saw this reduced version, ignore it.
    if (cache.isInCache(p))
      return res + " - Hitting cache";

    // Program ended up being larger, ignore it.
    if (newSize >= lastSize)
      return res + " - Mutation was bigger: " + std::to_string(newSize) +
             " vs old " + std::to_string(lastSize);

    // Check if the smaller version is actually interesting.
    Feedback f = feedback(p);
    if (!f.interesting)
      return res + " - Mutation not interesting";
    toReduce = p;
    triesLeft = maxTries;
    lastSize = newSize;
    return res;
  }

  /// Sets how many mutation tries this reducer should do before giving up.
  void setTries(unsigned t) {
    maxTries = t;
    triesLeft = t;
  }

  /// Returns the reduced program.
  const Program &getProgram() const { return toReduce; }
};

#endif // REDUCER_H
