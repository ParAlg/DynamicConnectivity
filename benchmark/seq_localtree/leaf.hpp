#pragma once
#include "assert.hpp"
#include <set>
#include <map>
#include <bitset>
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

class leaf {
 public:
  // using incident_edges = std::map<size_t, std::set<size_t>>;
  // using edge_lists = std::pair<size_t, std::set<size_t>>;

  leaf(size_t _id = 0) : E(), parent(nullptr), edgemap(), size(0), id(_id) {}
  bool insert(size_t e, size_t l);
  bool remove(size_t e, size_t l);
  size_t getLevel(size_t e);
  bool checkLevel(size_t l);
  void linkToRankTree(void *p);
  void *getParent() { return parent; }
  bool checkLevelEdge(size_t l) { return edgemap[l]; }
  size_t getSize() { return size; }
  size_t getID() { return id; }
  std::bitset<64> getEdgeMap() { return edgemap; }
  std::pair<std::set<size_t>::iterator, std::set<size_t>::iterator> getLevelIterator(size_t l);
  uint64_t space() {
    uint64_t space = 0;
    space += sizeof(std::map<size_t, std::set<size_t>>);
    space += E.size() * (sizeof(size_t) + sizeof(std::set<size_t>));
    for (auto entry : E)
      space += entry.second.size() * sizeof(size_t);
    space += sizeof(void*);
    space += sizeof(std::bitset<64>);
    space += 2 * sizeof(size_t);
    return space;
  }

 private:
  std::map<size_t, std::set<size_t>> E;
  void *parent;  // pointer to rank tree of the level logn cluster node
  std::bitset<64> edgemap;
  size_t id;
  size_t size;
};
inline void leaf::linkToRankTree(void *p) {
  parent = p;
}
inline bool leaf::insert(size_t e, size_t l) {
  auto &E = this->E;
  // find if there is level l incident edges
  // std::cout << "inserting " << e << " to level " << l << std::endl;
  size++;
  auto it = E.find(l);
  if (it != E.end()) {
    // insert to grouped BBST
    it->second.insert(e);
  } else {
    // insert to a new group
    std::set<size_t> e_;
    e_.insert(e);
    E.insert(std::make_pair(l, std::move(e_)));
    this->edgemap[l] = 1;
    return true;
  }
  return false;
  // std::cout << "after insertion, has " << this->E.size() << "levels, total number of edges is "
  //           << (this->E.find(l))->second.size() << std::endl;
}
inline bool leaf::remove(size_t e, size_t l) {
  auto &E = this->E;
  auto it = E.find(l);
  ASSERT_MSG(it != E.end(), "remove from wrong edge level");
  it->second.erase(e);
  if (it->second.size() == 0) {
    E.erase(it);
    this->edgemap[l] = 0;
    return true;
  }
  return false;
}
inline size_t leaf::getLevel(size_t e) {
  auto &E = this->E;
  size_t level = UINT64_MAX;
  bool flag = false;
  // std::cout << "number of levels " << E.size() << std::endl;
  for (auto const &[l, edges] : E) {
    // std::cout << "number of vertices in each level " << edges.size() << std::endl;
    if (edges.find(e) != edges.end()) {
      level = l;
      flag = true;
      break;
    }
  }

  // std::cout << "level is " << level << std::endl;
  ASSERT_MSG(flag == true, "edge doesn't exist");

  return level;
}
inline bool leaf::checkLevel(size_t l) {
  // check if this vertex has level l incident edges.
  auto &E = this->E;
  auto it = E.find(l);
  if (it != E.end()) return true;
  return false;
}
inline std::pair<std::set<size_t>::iterator, std::set<size_t>::iterator> leaf::getLevelIterator(size_t l) {
  return std::make_pair(this->E.find(l)->second.begin(), this->E.find(l)->second.end());
}