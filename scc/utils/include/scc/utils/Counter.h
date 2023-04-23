#ifndef COUNTER_H
#define COUNTER_H

#include <cstddef>

/// Support code for just counting.
namespace count {
struct CounterIterator;
struct Counter {
  typedef std::size_t Value;
  explicit Counter(Value v) : value(v) {}

  friend class CounterIterator;

private:
  Value value = 0;
};
struct CounterIterator {
  typedef Counter::Value Value;
  explicit CounterIterator(Value v) : counter(v) {}
  CounterIterator &operator++() {
    ++counter.value;
    return *this;
  }
  Counter operator*() const { return counter; }
  bool operator==(CounterIterator other) const {
    return other.counter.value == counter.value;
  }
  bool operator!=(CounterIterator other) const { return !(*this == other); }

private:
  Counter counter = Counter(0);
};

struct CounterRange {
  CounterRange(Counter::Value end) : endCounter(end) {}
  auto begin() { return CounterIterator(0); }
  auto end() { return endCounter; }

private:
  CounterIterator endCounter = CounterIterator(0);
};

/// Returns a counter range that does 'limit' iterations.
inline CounterRange upTo(Counter::Value limit) { return CounterRange(limit); }
} // namespace count

#endif
