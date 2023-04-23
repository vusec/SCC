#ifndef PROGRAMCACHE_H
#define PROGRAMCACHE_H

#include "scc/program/Program.h"

/// A cache of already seen programs.
///
/// This is used to avoid re-running if we hit exactly the same program twice.
class ProgramCache {
  // TODO: This class should use de Bruijn indices for equivalence.

  /// List of seen program hashes.
  std::unordered_set<HashStream::Hash> seenHashes;
  /// The max number of hashes to store.
  size_t maxHashes = 40000;
  /// Total number of queries.
  size_t queries = 0;
  /// How many queries hit the cache.
  size_t hashHits = 0;

public:
  /// Returns true if the program is in the cache.
  bool isInCacheNoInsert(const Program &p) const {
    HashStream s;
    OptError e = p.print(s);
    return seenHashes.count(s.getHash()) != 0;
  }

  /// Returns true if the program is in the cache. If it's not in the cache
  /// it is inserted.
  ///
  /// It's the callers responsibility to check that the program is in a
  /// printable state.
  bool isInCache(const Program &p) {
    ++queries;

    HashStream s;
    p.print(s).assumeSuccess("Failed to print program in hash");
    const HashStream::Hash hash = s.getHash();

    // Check if we already seen this hash.
    if (seenHashes.count(hash) != 0) {
      ++hashHits;
      return true;
    }

    seenHashes.insert(hash);
    // TODO: This is not as efficient as it should be.
    if (seenHashes.size() > maxHashes)
      seenHashes.clear();
    return false;
  }

  /// Return the number of times the cache was hit.
  size_t getCacheHits() const { return hashHits; }

  /// Returns the percentage chance (0-100) of how often the cache was hit.
  size_t getCacheHitRate() const { return hashHits * 100U / (queries + 1U); }
};

#endif // PROGRAMCACHE_H
