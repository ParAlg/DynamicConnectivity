#pragma once
#include "absl/container/flat_hash_set.h"
#include "graph.hpp"
#include <absl/container/btree_set.h>
#include <bitset>
#include <cassert>
#include <cstdint>
#include <cstring>
#include <tuple>
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

private:
  //   std::map<size_t, std::set<size_t>> E;
  // std::set<uint32_t> *E[MAX_LEVEL + 1];
  // std::vector<size_t> *E_[MAX_LEVEL + 1];
  // std::unordered_set<size_t> *E_[MAX_LEVEL + 1];
  absl::flat_hash_set<uint32_t> *E[MAX_LEVEL + 1];
  // edge_set *E[MAX_LEVEL + 1];
  void *parent; // pointer to rank tree of the level logn cluster node
  std::bitset<64> edgemap;
  vertex id;
  vertex size;
};
inline void leaf::linkToRankTree(void *p) { parent = p; }
inline void leaf::insert(vertex e, uint32_t l) {
  auto &E = this->E;
  // find if there is level l incident edges
  size++;
  // if (E[l] == nullptr) // no edge in this level
  //   E[l] = new std::set<size_t>;
  // assert(E[l]->find(e) == E[l]->end());
  // E[l]->insert(e);
  // if (E_[l] == nullptr)
  //   E_[l] = new std::vector<size_t>;
  // E_[l]->push_back(e);
  // if (E[l] == nullptr)
  //   E[l] = new edge_set();
  // E[l]->insert(e);
  if (E[l] == nullptr)
    E[l] = new absl::flat_hash_set<uint32_t>();
  E[l]->insert(e);
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
    delete E[l];
    E[l] = nullptr;
    this->edgemap[l] = 0;
  }
}
inline uint32_t leaf::getLevel(vertex e) {
  auto &E = this->E;
  // uint32_t level = MAX_LEVEL + 1;
  // bool flag = false;
  for (uint32_t i = 0; i <= MAX_LEVEL; i++) {
    if (E[i] != nullptr && E[i]->contains(e)) {
      // level = i;
      // flag = true;
      // break;
      return i;
    }
  }
  // if (flag == false)
  //   std::cout << "level is " << level << std::endl;
  // assert(flag == true);

  // return level;
  return MAX_LEVEL + 1;
}
inline bool leaf::checkLevel(uint32_t l) {
  // check if this vertex has level l incident edges.
  assert(this->edgemap[l] == (E[l] != nullptr && E[l]->empty()));
  return this->edgemap[l];
}