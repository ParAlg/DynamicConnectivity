#pragma once
#include <atomic>
#include <cstddef>
#include <parlay/primitives.h>
class localTree;
class stats {
 public:
  size_t level;
  size_t fanout;  // fanout
  size_t height;  // maximum height of rank tree
  size_t size;    // # incident vertices
  static std::atomic<size_t> memUsage;
  stats(size_t _l, size_t _f, size_t _h, size_t _s) : level(_l), fanout(_f), height(_h), size(_s) {}
};
inline std::atomic<size_t> stats::memUsage = 0;
class Radius {
 public:
  size_t found;
  size_t nRu;
  size_t nEu;
  size_t nCu;
  size_t nRv;
  size_t nEv;
  size_t nCv;
  size_t level;
  Radius(size_t _f, size_t _ru, size_t _eu, size_t _cu, size_t _rv, size_t _ev, size_t _cv, size_t _l) :
      found(_f), nRu(_ru), nEu(_eu), nCu(_cu), nRv(_rv), nEv(_ev), nCv(_cv), level(_l) {}
};
static parlay::sequence<Radius> Rstat;