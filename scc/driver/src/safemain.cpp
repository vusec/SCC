#include "ArgParser.h"
#include "Driver.h"
#include "SafeGenerator.h"
#include "Scheduler.h"

#include <iostream>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "not enough args\n";
    return 1;
  }

  ArgParser args;
  if (auto err = args.parse(argc, argv)) {
    std::cerr << "Failed to parse arguments: " << *err << "\n";
    std::cerr << "Usage: " << argv[0]
              << " [--tries=N] [--queue-size=N] [--scale=N] "
                 "-- oracle-bin oracle-arg1\n";
    return 1;
  }

  std::random_device d;
  size_t seed = d();

  Scheduler<SafeGenerator> sched(seed);
  sched.setMaxQueueSize(args.queueSize);
  sched.setMaxRunLimit(args.tries);
  sched.setMutatorScale(args.mutatorScale);
  sched.setStopAfter(args.stopAfter);

  Driver driver(
      sched, args.getEvalCommand(), [&sched]() { sched.step(); }, ".");
  driver.setUpdateInterval(args.uiUpdateMs);

  driver.run();
}
