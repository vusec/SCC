#ifndef SCHEDULERBASE_H
#define SCHEDULERBASE_H

#include <functional>
#include <list>

#include "ProgramCache.h"
#include "Rng.h"
#include "scc/program/Program.h"

/// Base class for the scheduler.
///
/// This contains everything that is not a template.
class SchedulerBase {
public:
  typedef int64_t Score;
  struct Feedback {
    explicit Feedback(Score score = -100000) : score(score) {}
    Score score = 0;
    bool interesting = false;
    bool deadEnd = false;
    std::string msg;
  };
  typedef std::function<Feedback(const Program &)> FeedbackFunc;

protected:
  /// The fitness function (the function to call on a program to give it a
  /// score).
  FeedbackFunc evalFunc;

  /// Rng used to select passes and generator seeds.
  Rng rng;

  /// Groups a program and the scheduling metadata.
  struct ProgAndMetadata {
    Program p;
    size_t programNodes = 0;
    size_t lengthGranularity = 1;
    std::string feedbackMsg;

    void setProgram(Program &&p) {
      this->p = p;
      updateLen();
    }

    void updateLen() { programNodes = p.countNodes(); }

    size_t sizeForSorting() const { return programNodes / lengthGranularity; }

    /// The score the fitness function gave this program.
    Score score = std::numeric_limits<Score>::min();
    /// How often we selected it as the base for mutations.
    size_t runs = 0;
    /// The message from the oracle for this one.
    std::string message;
  };

  /// The queue of candidates to mutate. Back of the queue
  /// is always the program with the highest score.
  std::list<ProgAndMetadata> queue;

  unsigned desperation = 1;
  void setDesperation(size_t i) {
    desperation = i;
    if (desperation == 0)
      desperation = 1;
  }

  /// The maximum amount of times a proram can serve as a
  /// candidate for mutation.
  size_t maxRunLimit = 100;
  /// How many programs to keep in the queue.
  size_t maxQueueSize = 300;

  /// Info string displayed to the user.
  std::string lastStratInfo;

  std::vector<Program> interestingResults;

  size_t numFindings = 0;

  unsigned mutatorScale = 1;

  size_t runsSinceProgress = 0;

  unsigned reducerTries = 4000;

protected:
  void addProgram(Program &&p) {
    ProgAndMetadata start;
    start.setProgram(std::move(p));
    queue.push_back(ProgAndMetadata(start));
  }

  void sortQueue() {
    for (ProgAndMetadata &p : queue)
      p.lengthGranularity = desperation;

    queue.sort([](const ProgAndMetadata &l, const ProgAndMetadata &r) {
      if (l.score == r.score)
        return l.sizeForSorting() > r.sizeForSorting();
      return l.score < r.score;
    });
    while (queue.size() > maxQueueSize)
      queue.pop_front();
  }

  bool resetRequest = true;

  // How many mutation->test iterations this scheduler did.
  size_t iterations = 0;
  // Same as iterations but does not count cache hits or malformed programs.
  size_t nonCacheIterations = 0;
  // After how many non-cached iterations this scheduler is finished.
  size_t stopAfterNIterations = std::numeric_limits<size_t>::max();
  // After how many interesting programs this scheduler is finished.
  size_t stopAfterHits = 0;

  ProgramCache cache;

  void informAboutRun(bool madeProgress) {
    if (madeProgress) {
      mutatorScale = 1;
      runsSinceProgress = 0;
      setDesperation(1);
      return;
    }
    mutatorScale = 1;
    ++runsSinceProgress;
    if (runsSinceProgress > 20000)
      setDesperation(32);
    if (runsSinceProgress > 40000)
      setDesperation(64);
    if (runsSinceProgress > 80000) {
      setDesperation(128);
      mutatorScale = 2;
    }
    if (runsSinceProgress > 180000) {
      mutatorScale = 3;
    }
    if (runsSinceProgress > 300000) {
      setDesperation(256);
      mutatorScale = 3;
    }
  }

public:
  /// Creates a scheduler that mutates a new program.
  SchedulerBase(FeedbackFunc feedback, uint64_t seed)
      : evalFunc(feedback), rng(RngSource(seed)) {}

  /// Creates a scheduler that mutates a new program.
  /// The created Scheduler here is dead, it needs a
  /// feedback function supplied before the first run.
  SchedulerBase(uint64_t seed) : rng(RngSource(seed)) {}

  Score getBestScore() const { return queue.back().score; }

  void setEvalFunction(FeedbackFunc f) { evalFunc = f; }

  size_t getNumFindings() const { return numFindings; }

  std::vector<Program> popInteresting() {
    auto res = interestingResults;
    interestingResults.clear();
    return res;
  }

  size_t getDesperation() const { return desperation; }

  void requestQueueReset() { resetRequest = true; }

  virtual const Program &getBestProg() {
    assert(!queue.empty());
    return queue.back().p;
  }

  virtual std::string getBestProcMsg() const { return ""; }

  virtual bool isReducing() const { return false; }

  std::string getLastStratInfo() const { return lastStratInfo; }

  void setMaxRunLimit(size_t v) { maxRunLimit = v; }

  void setMaxQueueSize(size_t v) { maxQueueSize = v; }

  void setMutatorScale(unsigned v) { mutatorScale = v; }

  void setReducerTries(unsigned i) { reducerTries = i; }

  bool finished() const {
    // If we're supposed to stop after a certain amount of findings then stop.
    if (stopAfterHits != 0 && numFindings >= stopAfterHits)
      return true;
    return nonCacheIterations >= stopAfterNIterations;
  }

  void setStopAfter(size_t v) { stopAfterNIterations = v; }

  void setStopAfterHit(std::size_t v) { stopAfterHits = v; }

  size_t getCacheHits() const { return cache.getCacheHits(); }

  size_t getCacheHitRate() const { return cache.getCacheHitRate(); }
};

#endif // SCHEDULERBASE_H
