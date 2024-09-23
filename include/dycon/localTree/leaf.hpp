#pragma once
#include "dycon/localTree/alloc.h"
#include "graph.hpp"
#include <absl/container/btree_map.h>
#include <absl/container/btree_set.h>
#include <atomic>
#include <bitset>
#include <cassert>
#include <cstddef>
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
      : parent(p), edgemap(0), size(0), id(_id) {
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
  edge_set *getLevelEdge(uint32_t l);
  std::pair<vertex, edge_set *> getLevelEdgeSet(uint32_t l);
  static type_allocator<edge_set> *vector_alloc;
  static size_t getLeafSpace(leaf *node);

private:
  std::vector<std::pair<uint32_t, edge_set *>> E;
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
  if (this->edgemap[l] == 1) {
    E[(edgemap << (63 - l)).count() - 1].second->insert(e);
    return;
  }

  this->edgemap[l] = 1;
  auto nghs = vector_alloc->create();
  nghs->emplace(e);
  auto setbit = (edgemap << (63 - l)).count();
  if (E.size() < setbit) {
    E.emplace_back(std::pair(l, nghs));
  } else {
    auto it = E.begin() + setbit - 1;
    E.insert(it, std::pair(l, nghs));
  }
}
inline std::tuple<bool, vertex, vertex> leaf::fetchEdge(uint32_t l) {
  auto nghs = E[(edgemap << (63 - l)).count() - 1].second;
  return std::make_tuple(true, id, *(nghs->begin()));
}
inline void leaf::remove(vertex e, uint32_t l) {
  this->size--;
  auto it = E.begin() + (edgemap << (63 - l)).count() - 1;
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
  return std::pair(id, E[(edgemap << (63 - l)).count() - 1].second);
}
inline edge_set *leaf::getLevelEdge(uint32_t l) {
  return E[(edgemap << (63 - l)).count() - 1].second;
}
inline size_t leaf::getLeafSpace(leaf *node) {
  if (!node)
    return 0;
  std::atomic<size_t> sz = 0;
  sz += sizeof(leaf);
  sz += sizeof(std::pair<uint32_t, edge_set *>) * node->E.size();
  sz += node->size * 5;
  return sz;
}