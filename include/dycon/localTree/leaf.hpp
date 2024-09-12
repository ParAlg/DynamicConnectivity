#pragma once
#include "absl/container/flat_hash_set.h"
#include "dycon/localTree/alloc.h"
#include "graph.hpp"
#include <absl/container/btree_set.h>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <vector>
// This is the leaf of the cluster forest.
// It contains a vertex in the graph and its incident edges.
// The incident edges are grouped by their level. So we need at
// most logn BBST for each vertex. However, we can prove that the
// total space will still be bounded by O(m) but not O(nlogn)
// The structure needs to support the following operation
// 1.Insert an edge to level l;
// 2.Delete an edge from level l;
// 3.Given an edge(u,v), use O(logn) to find the level of the edge.
// 4.Given a level l, use O(loglogn) to determine if there are any
// incident edges in that level
#define MAX_LEVEL 32
class leaf {
public:
  // using incident_edges = std::map<size_t, std::set<size_t>>;
  // using edge_lists = std::pair<size_t, std::set<size_t>>;

  leaf(vertex _id = 0, void *p = nullptr)
      : parent(p), edgemap(), size(0), id(_id) {
    memset(E, 0, sizeof(E));
  };
  void insert(vertex e, uint32_t l);
  void remove(vertex e, uint32_t l);
  uint32_t getLevel(uint32_t e);
  bool checkLevel(uint32_t l);
  void linkToRankTree(void *p);
  void *getParent() { return parent; }
  bool checkLevelEdge(uint32_t l) { return edgemap[l]; }
  vertex getSize() { return size; }
  vertex getID() { return id; }
  std::bitset<64> getEdgeMap() { return edgemap; }
  std::tuple<bool, vertex, vertex> fetchEdge(uint32_t l);
  static type_allocator<absl::flat_hash_set<vertex>> *vector_alloc;

private:
  absl::flat_hash_set<vertex> *E[MAX_LEVEL + 1];
  void *parent;
  std::bitset<64> edgemap;
  vertex id;
  vertex size;
};
inline type_allocator<absl::flat_hash_set<vertex>> *leaf::vector_alloc =
    nullptr;
inline void leaf::linkToRankTree(void *p) { parent = p; }
inline void leaf::insert(vertex e, uint32_t l) {
  auto &E = this->E;
  size++;
  if (E[l] == nullptr) {
    E[l] = vector_alloc->create();
  }
  E[l]->emplace(e);
  this->edgemap[l] = 1;
}
inline std::tuple<bool, vertex, vertex> leaf::fetchEdge(uint32_t l) {
  auto &E = this->E;
  if (E[l] == nullptr || E[l]->empty())
    return std::make_tuple(false, 0, 0);
  auto e = std::make_tuple(true, id, *(E[l]->begin()));
  return e;
}
inline void leaf::remove(vertex e, uint32_t l) {
  this->size--;
  auto &E = this->E;
  assert(E[l] != nullptr && !E[l]->empty());
  E[l]->erase(e);
  if (E[l]->empty()) {
    vector_alloc->free(E[l]);
    E[l] = nullptr;
    this->edgemap[l] = 0;
  }
}
inline uint32_t leaf::getLevel(vertex e) {
  auto &E = this->E;
  for (uint32_t i = 0; i <= MAX_LEVEL; i++) {
    if (E[i] != nullptr && E[i]->contains(e)) {
      return i;
    }
  }
  return MAX_LEVEL + 2;
}
inline bool leaf::checkLevel(uint32_t l) {
  // check if this vertex has level l incident edges.
  assert(this->edgemap[l] == (E[l] != nullptr && E[l]->empty()));
  return this->edgemap[l];
}