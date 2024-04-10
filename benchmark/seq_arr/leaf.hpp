#pragma once
#include "assert.hpp"
#include <set>
#include <map>
#include <bitset>
class leaf {
 public:
  leaf(size_t _id = 0, void *p = nullptr) : E(), parent(p), edgemap(0), size(0), id(_id) {}
  ~leaf() { E.clear(); }
  bool insert(size_t e, size_t l);
  bool remove(size_t e, size_t l);
  size_t getLevel(size_t e);
  bool checkLevel(size_t l);
  void linkToCluterForest(void *p);
  void *getParent() { return parent; }
  bool checkLevelEdge(size_t l) { return edgemap[l]; }
  size_t getSize() { return size; }
  size_t getID() { return id; }
  std::bitset<64> getEdgeMap() { return edgemap; }
  std::pair<std::set<size_t>::iterator, std::set<size_t>::iterator> getLevelIterator(size_t l);

 private:
  std::map<size_t, std::set<size_t>> E;
  void *parent;
  std::bitset<64> edgemap;  // let edgemap indicates the id of the vertices at level 0
  size_t id;
  size_t size;
};
inline void leaf::linkToCluterForest(void *p) {
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
  auto &E = this->E;
  auto it = E.find(l);
  if (it != E.end()) return true;
  return false;
}
inline std::pair<std::set<size_t>::iterator, std::set<size_t>::iterator> leaf::getLevelIterator(size_t l) {
  return std::make_pair(this->E.find(l)->second.begin(), this->E.find(l)->second.end());
}