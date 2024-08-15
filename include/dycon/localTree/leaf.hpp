#ifndef LT_LEAF
#define LT_LEAF
#include "edgeset.hpp"
#include <bitset>
#include <cassert>
#include <cstddef>
#include <map>
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
const size_t MAX_LEVEL = 33;
class leaf {
public:
  leaf(size_t _id = 0, void *p = nullptr)
      : parent(p), edgemap(), size(0), id(_id) {
    // memset(E, 0, sizeof(DynamicArray<size_t, 64> *) * 32);
    for (size_t i = 0; i < MAX_LEVEL; i++)
      E[i] = new DynamicArray<size_t, 64>;
  }
  ~leaf() {
    for (size_t i = 0; i < MAX_LEVEL; i++)
      delete E[i];
  }
  void insert(size_t e, size_t l);
  void remove_lazy(size_t e, size_t l);
  void remove(size_t e, size_t ith, size_t l);
  std::pair<size_t, size_t> getEdgeInfo(size_t e);
  bool hasLevelEdge(size_t l);
  void flattenLevel(size_t l) { E[l]->print_all(); }
  size_t getNumEdges(size_t l) { return E[l]->get_size(); }
  std::tuple<bool, size_t, size_t> fetchEdge(size_t l);
  void linkToRankTree(void *p) { parent = p; }
  void *getParent() { return parent; }
  bool hasLevelEdgeEdge(size_t l) { return edgemap[l]; }
  size_t getSize() { return size; }
  size_t getID() { return id; }
  std::bitset<64> getEdgeMap() { return edgemap; }
  void flatten();

private:
  size_t size;
  // store level of edge (id,v)
  // map<id,<level,ith in array>>;
  std::map<size_t, std::pair<size_t, size_t>> L;
  void *parent; // pointer to rank tree of the level logn cluster node
  std::bitset<64> edgemap;
  size_t id;
  DynamicArray<size_t, 64> *E[MAX_LEVEL];
};
inline void leaf::insert(size_t e, size_t l) {
  this->size++;    // #incident vertices
  E[l]->insert(e); // add to dynamic array
  this->edgemap[l] = 1;
  //  <other_endpoint,<level,ith in the array>>
  this->L.insert(std::make_pair(e, std::make_pair(l, E[l]->get_size())));
}
// remove the last one from the array
inline void leaf::remove_lazy(size_t e, size_t l) {
  if (E[l]->tail() != e) {
    std::cout << this->id << " " << e << " " << E[l]->tail() << " " << " "
              << E[l]->get_size() << std::endl;
    flatten();
  }

  assert(e == E[l]->tail()); // delete the correct edge
  this->size--;              // #incident vertices
  this->L.erase(e);          // not incident vertex anymore
  E[l]->pop();               // pop from tail of the array
  if (E[l]->is_empty())      // bitmap
    this->edgemap[l] = 0;
}
inline void leaf::remove(size_t e, size_t ith, size_t l) {
  if (E[l]->at(ith) != e) {
    std::cout << this->id << " " << e << " " << E[l]->at(ith) << " " << ith
              << " " << E[l]->get_size() << std::endl;
    E[l]->print_all();
  }
  // since we swap the ith to the E[l]->num_elementh
  //  we also need to update the ith info for the new one at ith
  assert(E[l]->at(ith) == e); // delete the correct edge
  L[E[l]->tail()] = std::make_pair(l, ith);
  // L[e] = std::make_pair(l, E[l]->get_size());
  this->size--;      // #incident vertices
  this->L.erase(e);  // not incident vertex anymore
  E[l]->remove(ith); // remove specific element
  if (E[l]->is_empty())
    this->edgemap[l] = 0;
}

inline std::pair<size_t, size_t> leaf::getEdgeInfo(size_t e) {
  auto it = L.find(e);
  assert(it != L.end());
  return it->second;
}
inline bool leaf::hasLevelEdge(size_t l) {
  // check if this vertex has level l incident edges.
  assert(!(E[l]->is_empty()) == this->edgemap[l]);
  return !E[l]->is_empty();
}
inline std::tuple<bool, size_t, size_t> leaf::fetchEdge(size_t l) {
  if (this->edgemap[l] == 0)
    return std::make_tuple(false, 0, 0);
  return std::make_tuple(true, id, E[l]->tail());
}
inline void leaf::flatten() {
  for (size_t i = 0; i < MAX_LEVEL; i++) {
    if (this->edgemap[i]) {
      std::cout << "flatten level " << i << " edges\n";
      E[i]->print_all();
    }
  }
}
#endif