#pragma once
#include "rankTree.hpp"
#include "rankArr.hpp"
#include "leaf.hpp"
#include "stats.hpp"
#include <parlay/primitives.h>
#include <parlay/sequence.h>
class rankTree;
class rankArr;
class localTree {
 private:
  friend class rankTree;
  friend class rankArr;
  using nodeArr = parlay::sequence<localTree*>;
  // Each time manipulating node, check if 6 members are covered.
  parlay::sequence<rankTree*> rTrees;
  size_t level;
  size_t size;  // number of vertices incident to this cluster node.
  rankTree* parent;
  leaf* vertex;
  // aug_value
  std::bitset<64> edgemap;
  localTree* min_child;
  localTree* max_child;
  static void updateBitMap(localTree* node);
  static bool comp(localTree* x, localTree* y) { return x->size < y->size; }

 public:
  localTree(size_t _id) :
      level(0),
      size(1),
      parent(nullptr),
      vertex(new leaf(_id, this)),
      edgemap(0),
      max_child(nullptr),
      min_child(nullptr) {
    rTrees.clear();
  }
  localTree() :
      level(0), size(0), parent(nullptr), vertex(nullptr), edgemap(0), max_child(nullptr), min_child(nullptr) {
    rTrees.clear();
  };
  localTree(localTree* Cu, localTree* Cv);
  ~localTree();
  size_t getLevel() { return this->level; }
  std::bitset<64> getMap() { return this->edgemap; }
  void setLevel(size_t l) {
    assert((1 << l) >= this->size);
    this->level = l;
  }
  size_t getSize() { return this->size; }
  void setSize(size_t sz) { this->size = sz; }
  static void merge(localTree* Cu, localTree* Cv);
  static void addChild(localTree* p, localTree* son);
  static void add2Children(localTree* p, localTree* s1, localTree* s2);
  static void deleteFromParent(localTree* p);
  static localTree* getParent(localTree* r);
  static localTree* getRoot(localTree* r);
  static localTree* getLevelNode(localTree* r, size_t l);
  static nodeArr getRootPath(localTree* r);
  static size_t getRootPathLen(localTree* r, localTree** p);
  void insertToLeaf(size_t v, size_t l);
  void deleteEdge(size_t v, size_t l);
  size_t getEdgeLevel(size_t e);
  static void traverseTopDown(localTree* root, bool clear, bool verbose, bool stat, parlay::sequence<stats>& info);
  static std::tuple<bool, size_t, size_t> fetchEdge(localTree* root, size_t l);
  static localTree* getIfSingleton(localTree* r);
  static bool ifSingleton(localTree* r);
  static localTree* fetchSmallest(localTree* node);
  static localTree* fetchGreatest(localTree* node);
};
inline localTree::~localTree() {
  if (vertex) delete vertex;
}
inline localTree::localTree(localTree* Cu, localTree* Cv) {
  assert(Cu->level >= Cv->level);
  this->level = Cu->level + 1;
  this->size = Cu->size + Cv->size;
  this->parent = nullptr;
  this->vertex = nullptr;
  // aug_value
  this->edgemap = Cu->edgemap | Cv->edgemap;
  this->min_child = Cu->size < Cv->size ? Cu : Cv;
  this->max_child = Cu->size > Cv->size ? Cu : Cv;
  assert((1 << Cu->level) >= Cu->size);
  assert((1 << Cv->level) >= Cv->size);
  auto ru = new rankTree(std::log2(Cu->size), this, Cu, Cu->edgemap);
  auto rv = new rankTree(std::log2(Cv->size), this, Cv, Cv->edgemap);
  Cu->parent = ru;
  Cv->parent = rv;
  this->rTrees.clear();
  this->rTrees.push_back(rv);
  this->rTrees.push_back(ru);
  this->rTrees = rankTree::build(this->rTrees, this);
  assert(this->size <= 1 << this->level);
}
inline localTree* localTree::getIfSingleton(localTree* r) {
  if (r->rTrees.size() > 1) return r;
  if (!r->rTrees[0]->isleaf()) return r;
  auto node = r->rTrees[0]->descendant;
  if (node->level == 0) return r;
  delete node->parent;
  // node->parent = nullptr;
  // delete r;
  r->rTrees = node->rTrees;
  for (auto it : r->rTrees)
    it->node = r;
  delete node;
  return r;
}
inline bool localTree::ifSingleton(localTree* r) {
  if (!r) return false;
  if (r->rTrees.empty()) return true;
  if (r->rTrees.size() == 1 && r->rTrees[0]->isleaf())
    return true;
  else
    return false;
}
inline void localTree::merge(localTree* Cu, localTree* Cv) {
  // parent level vertex
  Cu->size = Cu->size + Cv->size;
  // aug_value
  Cu->min_child = Cu->min_child->size < Cv->min_child->size ? Cu->min_child : Cv->min_child;
  Cu->max_child = Cu->max_child->size > Cv->max_child->size ? Cu->max_child : Cv->max_child;
  Cu->rTrees = rankTree::Merge(Cu->rTrees, Cv->rTrees, Cu);
  Cu->edgemap = Cu->edgemap | Cv->edgemap;
  assert(Cu->level == Cv->level);
  assert(1 << Cu->level >= Cu->size);
  delete Cv;
  updateBitMap(Cu);
}
inline void localTree::addChild(localTree* p, localTree* son) {
  if (!p) return;

  auto rTree = new rankTree(std::log2(son->size), p, son, son->edgemap);
  son->parent = rTree;
  p->size += son->size;
  p->rTrees = rankTree::insertRankTree(p->rTrees, rTree, p);
  // aug_value
  p->edgemap = p->edgemap | son->edgemap;
  p->min_child = rankTree::getSmallest(p->rTrees);
  p->max_child = rankTree::getGreatest(p->rTrees);
  updateBitMap(p);
  // p->level = std::ceil(std::log2(p->size));
  assert(p->level > son->level);
  // std::cout << "level " << p->level << " size " << p->size << std::endl;
  assert((1 << p->level) >= p->size);
}
inline void localTree::add2Children(localTree* p, localTree* s1, localTree* s2) {
  if (!p) return;
  auto r1 = new rankTree(std::log2(s1->size), p, s1, s1->edgemap);
  s1->parent = r1;
  p->size += s1->size;
  p->rTrees = rankTree::insertRankTree(p->rTrees, r1, p);
  p->edgemap = p->edgemap | s1->edgemap;
  auto r2 = new rankTree(std::log2(s2->size), p, s2, s2->edgemap);
  s2->parent = r2;
  p->size += s2->size;
  p->rTrees = rankTree::insertRankTree(p->rTrees, r2, p);
  p->edgemap = p->edgemap | s2->edgemap;
  p->min_child = rankTree::getSmallest(p->rTrees);
  p->max_child = rankTree::getGreatest(p->rTrees);
  // p->level = std::ceil(std::log2(p->size));
  updateBitMap(p);
  if (p->level <= std::max(s1->level, s2->level) || (1 << p->level) < p->size)
    std::cout << "assert " << p->level << " " << p->size << " " << s1->level << " " << s1->size << " " << s2->level
              << " " << s2->size << std::endl;
  assert(p->level > std::max(s1->level, s2->level));
  assert((1 << p->level) >= p->size);
}
inline localTree* localTree::getParent(localTree* r) {
  if (!r->parent) return nullptr;
  auto p = rankTree::getRoot(r->parent);
  return p->node;
}
inline localTree* localTree::getRoot(localTree* r) {
  while (r->parent)
    r = getParent(r);
  return r;
}
// A level i edge connects level i-1 node Cu and Cv, return
inline localTree* localTree::getLevelNode(localTree* r, size_t l) {
  while (r->parent) {
    auto p = getParent(r);
    if (p->level == l) return r;
    r = p;
  }
  return nullptr;
}
inline localTree::nodeArr localTree::getRootPath(localTree* r) {
  nodeArr p;
  p.push_back(r);
  while (r->parent) {
    r = getParent(r);
    p.push_back(r);
  }
  return p;
}
inline size_t localTree::getRootPathLen(localTree* r, localTree** p) {
  size_t len = 0;
  while (r) {
    p[len++] = r;
    r = getParent(r);
  }
  return len;
}
inline void localTree::updateBitMap(localTree* node) {
  while (node->parent) {
    node = rankTree::updateBitMapByTree(node->parent, node->edgemap);
    if (!node) break;
    auto oval = node->edgemap;
    std::bitset<64> nval = 0;
    for (auto it : node->rTrees)
      nval = nval | it->edgemap;
    if (nval == oval) break;
    node->edgemap = nval;
  }
}
inline void localTree::insertToLeaf(size_t v, size_t l) {
  assert(this->vertex != nullptr);
  this->vertex->insert(v, l);
  this->edgemap = this->vertex->getEdgeMap();
  // go up to update
  updateBitMap(this);
}
inline void localTree::deleteEdge(size_t v, size_t l) {
  this->vertex->remove(v, l);
  this->edgemap = this->vertex->getEdgeMap();
  updateBitMap(this);
}
inline void localTree::deleteFromParent(localTree* p) {
  // remove p from its parent
  if (!p->parent) return;
  auto node = getParent(p);
  node->size -= p->size;
  auto oval = node->edgemap;
  node->rTrees = rankTree::remove(node->rTrees, p->parent, node);
  node->edgemap = rankTree::getEdgeMap(node->rTrees);
  if (node->edgemap != oval) updateBitMap(node);
  node->min_child = node->rTrees.empty() ? nullptr : rankTree::getSmallest(node->rTrees);
  node->max_child = node->rTrees.empty() ? nullptr : rankTree::getGreatest(node->rTrees);
  p->parent = nullptr;
}
inline size_t localTree::getEdgeLevel(size_t e) {
  return this->vertex->getLevel(e);
}
inline void localTree::traverseTopDown(localTree* root, bool clear, bool verbose, bool stat,
                                       parlay::sequence<stats>& info) {
  if (!root) return;
  stats::memUsage += sizeof(localTree);
  parlay::sequence<localTree*> children;
  while (!root->rTrees.empty()) {
    auto r = fetchGreatest(root);
    children.push_back(r);
  }
  for (size_t i = 0; i < children.size() - 1; i++)
    assert(children[i]->size >= children[i + 1]->size);
  for (auto it : children) {
    traverseTopDown(it, clear, verbose, stat, info);
    // if (clear) delete it;
  }
  if (clear) delete root;
}
inline std::tuple<bool, size_t, size_t> localTree::fetchEdge(localTree* root, size_t l) {
  if (root->level == 0) {
    assert(root->vertex != nullptr);
    return root->vertex->fetchEdge(l);
  }
  for (auto it : root->rTrees) {
    if (it->edgemap[l] == 1) {
      return fetchEdge(rankTree::fetchLeaf(it, l)->descendant, l);
    }
  }
  return std::make_tuple(false, 0, 0);
}
inline localTree* localTree::fetchSmallest(localTree* node) {
  auto r = node->min_child;
  deleteFromParent(r);
  return r;
}
inline localTree* localTree::fetchGreatest(localTree* node) {
  auto r = node->max_child;
  deleteFromParent(r);
  return r;
}
// friend class function
inline size_t rankTree::getLeafSize(rankTree* r) {
  return r->descendant->size;
}
inline localTree* rankTree::getLTreeTest() {
  return new localTree();
}