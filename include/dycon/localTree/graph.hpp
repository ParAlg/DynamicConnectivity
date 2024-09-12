#ifndef GRAPH
#define GRAPH
#include <cstdint>
using vertex = uint32_t;
struct Edge_info {
  vertex idu;
  vertex idv;
  uint32_t level;
};
#endif