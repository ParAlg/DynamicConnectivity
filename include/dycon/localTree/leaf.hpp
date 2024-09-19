#pragma once
#include "dycon/localTree/alloc.h"
#include "graph.hpp"
#include <absl/container/btree_map.h>
#include <absl/container/btree_set.h>
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
  std::bitset<64> changeLevel(std::vector<::vertex> &nghs, uint32_t oval,
                              uint32_t nval);
  vertex getSize() { return size; }
  vertex getID() { return id; }
  std::bitset<64> getEdgeMap() { return edgemap; }
  std::tuple<bool, vertex, vertex> fetchEdge(uint32_t l);
  // absl::flat_hash_set<vertex> *getLevelEdge(uint32_t l);
  edge_set *getLevelEdge(uint32_t l);
  // std::pair<vertex, absl::flat_hash_set<vertex> *> getLevelEdgeSet(uint32_t
  // l);
  std::pair<vertex, edge_set *> getLevelEdgeSet(uint32_t l);
  // static type_allocator<absl::flat_hash_set<vertex>> *vector_alloc;
  static type_allocator<edge_set> *vector_alloc;

private:
  absl::btree_map<uint16_t, edge_set *> E;
  void *parent;
  std::bitset<64> edgemap;
  vertex id;
  vertex size;
};
inline type_allocator<edge_set> *leaf::vector_alloc = nullptr;
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
}
inline uint32_t leaf::getLevel(vertex e) {
  auto &E = this->E;
  for (auto it : E)
    if (it.second->contains(e))
      return (uint32_t)it.first;
  return MAX_LEVEL + 2;
}
inline bool leaf::checkLevel(uint32_t l) { return this->edgemap[l]; }
inline std::pair<vertex, edge_set *> leaf::getLevelEdgeSet(uint32_t l) {
  auto e = E.find(l);
  assert(e != E.end());
  return std::pair(id, e->second);
}
inline edge_set *leaf::getLevelEdge(uint32_t l) {
  auto e = E.find(l);
  assert(e != E.end());
  return e->second;
}
inline std::bitset<64> leaf::changeLevel(std::vector<::vertex> &nghs,
                                         uint32_t oval, uint32_t nval) {
  auto oit = E.find(oval);
  auto nit = E.find(nval);
  assert(oit != E.end() && nghs.size() <= oit->second->size());
  if (nghs.size() == oit->second->size()) {
    this->edgemap[oval] = 0;
    this->edgemap[nval] = 1;
    if (nit == E.end()) {
      E.emplace(nval, oit->second);
    } else {
      auto eset = nit->second;
      for (auto it : nghs)
        eset->emplace(it);
      vector_alloc->free(oit->second);
    }
    E.erase(oval);
    return this->edgemap;
  }
  this->edgemap[nval] = 1;
  edge_set *neset;
  edge_set *oeset = oit->second;
  if (nit == E.end()) {
    neset = vector_alloc->create();
    E.emplace(nval, neset);
  } else
    neset = nit->second;
  for (auto it : nghs) {
    neset->emplace(it);
    oeset->erase(it);
  }
  return this->edgemap;
}