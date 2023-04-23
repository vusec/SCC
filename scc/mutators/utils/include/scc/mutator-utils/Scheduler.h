#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <deque>

#include "Reducer.h"
#include "SchedulerBase.h"

/// Schedules mutations on a target program.
template <typename GeneratorT> class Scheduler : public SchedulerBase {
  typedef typename GeneratorT::Strategy Strategy;

private:
  GeneratorT gen;
  /// The last Strategy used to mutate a program.
  Strategy *lastStrat = nullptr;

  LangOpts opts;

  /// Groups a strategy and the scheduling metadata.
  struct StratAndMetadata {
    Strategy strat;
    /// How many points we have gained by using this
    /// mutation strategy.
    size_t scoreGained = 0;
    /// How often this strategy has been applied.
    size_t runs = 1;

    StratAndMetadata() = default;
    StratAndMetadata(Strategy s) : strat(s) {}

    /// When picking a random strategy, how much weight
    /// should ge given to this program.
    size_t getPickWeight() const {
      return std::max<size_t>(1, scoreGained * 1000 / runs);
    }

    /// Called when this strategy has been used to do
    /// a mutation.
    /// \param gained How many points we gained by using
    ///        this strategy (or 0 if we gained no additional points)
    void didRun(size_t gained) {
      ++runs;
      scoreGained += gained;
    }

    void hitCache() {
      ++runs;
      if (scoreGained == 0)
        return;
      scoreGained -= 1;
    }

    bool operator<(const StratAndMetadata &o) const {
      return getPickWeight() < o.getPickWeight();
    }
  };
  std::vector<StratAndMetadata> strategies;

  static constexpr unsigned pickDifferentStratEvery = 8;

  /// Pick a random mutation strategy by using the assigned
  /// weights of each strategy.
  StratAndMetadata &pickStrat() {
    if (iterations % pickDifferentStratEvery == 0)
      return rng.pickOneVec(strategies);

    size_t selected = rng.getBelow<size_t>(totalStratWeight());
    size_t index = 0;
    for (auto &s : strategies) {
      const size_t weight = s.getPickWeight();
      if (weight >= selected) {
        lastStrat = &s.strat;
        return s;
      }
      selected -= weight;
      ++index;
    }
    assert(false && "Didn't select any strategy?");
    return strategies.back();
  }

  void init() {
    for (const auto &strat : Strategy::makeMutateStrategies())
      strategies.emplace_back(strat);
  }

  size_t totalStratWeight() const {
    size_t weightSum = 0;
    for (const auto &s : strategies)
      weightSum += s.getPickWeight();
    return weightSum;
  }

  size_t getRandomSeed() { return rng.makeSeed(); }

  void resetQueueToStart() {
    queue.clear();
    const unsigned initQueueSize = 3;
    for (unsigned i = 0; i < initQueueSize; ++i)
      addProgram(std::move(*gen.generate(getRandomSeed(), opts)));
    sortQueue();
  }

  std::unique_ptr<Reducer<GeneratorT>> reducer;

  void evaluateStrat(StratAndMetadata &strat, size_t points) {
    strat.didRun(points);
    informAboutRun(points > 0);
  }

public:
  Scheduler(FeedbackFunc feedback, uint64_t seed)
      : SchedulerBase(feedback, seed) {
    resetRequest = true;
    init();
  }

  Scheduler(FeedbackFunc feedback, uint64_t seed, Program p)
      : SchedulerBase(feedback, seed) {
    addProgram(std::move(p));
    init();
  }

  Scheduler(uint64_t seed, LangOpts o) : SchedulerBase(seed) {
    this->opts = o;
    init();
    queue = {ProgAndMetadata()};
    resetRequest = true;
  }

  OptError handleArgs(std::vector<std::string> args) {
    return gen.handleArgs(args);
  }

  /// Utility func that just calls `step` N times.
  void steps(unsigned num) {
    for (unsigned i = 0; i < num; ++i)
      step();
  }

  /// Utility func that steps until the first finding or 'max' iterations have
  /// been done.
  bool stepUntilFinding(unsigned max) {
    for (unsigned i = 0; i < max; ++i) {
      step();
      if (!interestingResults.empty())
        return true;
    }
    return false;
  }

  /// Do one mutation->eval step.
  void step() {
    if (finished())
      return;
    ++iterations;

    if (reducer) {
      if (reducer->finished()) {
        numFindings += 1;
        interestingResults.push_back(reducer->getProgram());
        reducer.reset();
        return;
      }
      lastStratInfo = reducer->step();
      return;
    }

    if (resetRequest) {
      resetRequest = false;
      resetQueueToStart();
    }

    if (queue.empty())
      resetQueueToStart();

    ProgAndMetadata mutationBase = queue.back();
    StratAndMetadata &strat = pickStrat();

    queue.back().runs += 1;
    if (queue.back().runs > maxRunLimit)
      queue.pop_back();

    if (queue.empty())
      resetQueueToStart();

    auto usedScale = std::max<unsigned>(1U, rng.getBelow(mutatorScale));
    Program p = mutationBase.p;
    gen.mutate(p, getRandomSeed(), strat.strat, usedScale);

    if (!p.canPrint() || cache.isInCache(p)) {
      evaluateStrat(strat, 0);
      return;
    }

    nonCacheIterations++;

    auto padTo = [](unsigned size, std::string &s) {
      if (s.size() >= size)
        return;
      s.resize(size, ' ');
    };
    Feedback mutationFeedback = evalFunc(p);
    lastStratInfo = std::string(strat.strat.getName());
    padTo(21, lastStratInfo);
    lastStratInfo += " (Score: ";
    lastStratInfo += std::to_string(strat.getPickWeight());
    padTo(37, lastStratInfo);
    lastStratInfo += " - Chance: ";
    lastStratInfo +=
        std::to_string(100 * strat.getPickWeight() / totalStratWeight()) + "%)";

    if (mutationFeedback.interesting) {
      lastStratInfo = "Reducing...";
      reducer.reset(new Reducer<GeneratorT>(evalFunc, rng.makeSeed(), p));
      reducer->setTries(reducerTries);
      resetQueueToStart();
      return;
    }

    if (mutationFeedback.deadEnd) {
      resetQueueToStart();
      return;
    }

    ProgAndMetadata newQueueElem;
    newQueueElem.setProgram(std::move(p));
    newQueueElem.score = mutationFeedback.score;
    newQueueElem.message = mutationFeedback.msg;

    if (mutationFeedback.score > mutationBase.score) {
      evaluateStrat(strat, 10);
    } else if (mutationFeedback.score == mutationBase.score &&
               mutationBase.sizeForSorting() > newQueueElem.sizeForSorting()) {
      evaluateStrat(strat, 10);
    } else {
      evaluateStrat(strat, 0);
      return;
    }

    queue.emplace_back(newQueueElem);

    sortQueue();
  }

  bool isReducing() const override { return reducer.get() != nullptr; }

  const Program &getBestProg() override {
    if (reducer)
      return reducer->getProgram();
    SCCAssert(!queue.empty(), "No best program available?");
    return queue.back().p;
  }

  std::string getBestProcMsg() const override {
    if (queue.empty())
      return "";
    return queue.back().message;
  }

  Strategy *getLastStrat() const { return lastStrat; }

  GeneratorT &getGenerator() { return gen; }
};

#endif // SCHEDULER_H
