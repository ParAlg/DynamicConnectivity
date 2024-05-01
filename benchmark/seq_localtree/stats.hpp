#pragma once
#include <atomic>
#include <cstddef>
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