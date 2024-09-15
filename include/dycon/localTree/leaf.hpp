#pragma once
#include "absl/container/flat_hash_set.h"
#include "dycon/localTree/alloc.h"
#include "graph.hpp"
#include <absl/container/btree_map.h>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <tuple>
#include <utility>
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
    E.clear();
  };
  void insert(vertex e, uint32_t l);
  void remove(vertex e, uint32_t l);
  uint32_t getLevel(uint32_t e);
  bool checkLevel(uint32_t l);
  void linkToRankTree(void *p);
  void *getParent() { return parent; }
  void setLevel(uint32_t l, bool val) { edgemap[l] = val; }
  bool checkLevelEdge(uint32_t l) { return edgemap[l]; }
  vertex getSize() { return size; }
  vertex getID() { return id; }
  std::bitset<64> getEdgeMap() { return edgemap; }
  std::tuple<bool, vertex, vertex> fetchEdge(uint32_t l);
  absl::flat_hash_set<vertex> *getLevelEdge(uint32_t l);
  std::pair<vertex, absl::flat_hash_set<vertex> *> getLevelEdgeSet(uint32_t l);
  static type_allocator<absl::flat_hash_set<vertex>> *vector_alloc;

private:
  // absl::flat_hash_set<vertex> *E[MAX_LEVEL + 1];
  absl::btree_map<uint16_t, absl::flat_hash_set<vertex> *> E;
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
  auto it = E.find((uint16_t)l);
  if (it == E.end()) {
    this->edgemap[l] = 1;
    auto hset = vector_alloc->create();
    hset->emplace(e);
    E.emplace(l, hset);
    return;
  }
  it->second->emplace(e);
}
inline std::tuple<bool, vertex, vertex> leaf::fetchEdge(uint32_t l) {
  auto it = E.find((uint16_t)l);
  if (it == E.end() || it->second->empty())
    return std::make_tuple(false, 0, 0);
  // if (E[l] == nullptr || E[l]->empty())
  //   return std::make_tuple(false, 0, 0);
  // auto e = std::make_tuple(true, id, *(E[l]->begin()));
  // return e;
  return std::make_tuple(true, id, *(it->second->begin()));
}
inline void leaf::remove(vertex e, uint32_t l) {
  this->size--;
  auto it = E.find((uint16_t)l);
  it->second->erase(e);
  if (it->second->empty()) {
    vector_alloc->free(it->second);
    E.erase(it);
    edgemap[l] = 0;
  }
  // auto &E = this->E;
  // assert(E[l] != nullptr && !E[l]->empty());
  // E[l]->erase(e);
  // if (E[l]->empty()) {
  //   vector_alloc->free(E[l]);
  //   E[l] = nullptr;
  //   this->edgemap[l] = 0;
  // }
}
inline uint32_t leaf::getLevel(vertex e) {
  auto &E = this->E;
  for (auto it : E)
    if (it.second->contains(e))
      return (uint32_t)it.first;
  // for (uint32_t i = 0; i <= MAX_LEVEL; i++) {
  //   if (E[i] != nullptr && E[i]->contains(e)) {
  //     return i;
  //   }
  // }
  return MAX_LEVEL + 2;
}
inline bool leaf::checkLevel(uint32_t l) {
  // check if this vertex has level l incident edges.
  // assert(this->edgemap[l] == (E[l] != nullptr && E[l]->empty()));
  return this->edgemap[l];
}
inline std::pair<vertex, absl::flat_hash_set<vertex> *>
leaf::getLevelEdgeSet(uint32_t l) {
  auto e = E.find(l);
  assert(e != E.end());
  return std::pair(id, e->second);
}
inline absl::flat_hash_set<vertex> *leaf::getLevelEdge(uint32_t l) {
  auto e = E.find(l);
  assert(e != E.end());
  return e->second;
}