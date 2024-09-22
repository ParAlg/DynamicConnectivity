#ifndef GRAPH
#define GRAPH
#include <absl/container/btree_set.h>
#include <cstdint>
using vertex = uint32_t;
using edge_set = absl::btree_set<vertex>;
#endif