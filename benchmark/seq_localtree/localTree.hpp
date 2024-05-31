#pragma once
#include "rankTree.hpp"
#include "rankArr.hpp"
#include "leaf.hpp"
#include "stats.hpp"
#include <parlay/primitives.h>
#include <parlay/sequence.h>
class rankTree;
class rankArr;
inline size_t fanout[10000001];
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
  std::bitset<64> edgemap;
  static void updateBitMap(localTree* node);
  template<localTree*>
  static bool comp(localTree* x, localTree* y) {
    return x->size < y->size;
  }

 public:
  localTree(size_t _id) : level(0), size(1), parent(nullptr), vertex(new leaf(_id, this)), edgemap(0) {
    rTrees.clear();
  }
  localTree(nodeArr& Q);
  localTree() : level(0), size(0), parent(nullptr), vertex(nullptr), edgemap(0) { rTrees.clear(); };
  localTree(localTree* Cu, localTree* Cv);
  ~localTree();
  size_t getLevel() { return this->level; }
  std::bitset<64> getMap() { return this->edgemap; }
  void setLevel(size_t l) {
    assert((1 << l) >= this->size);
    this->level = l;
  }
  size_t getSize() { return this->size; }
  static void merge(localTree* Cu, localTree* Cv);
  static localTree* mergeNode(nodeArr& Q, size_t l);
  static void addChild(localTree* p, localTree* son);
  static void add2Children(localTree* p, localTree* s1, localTree* s2);
  static void deleteFromParent(localTree* p);
  static size_t getRTLen(localTree* p) { return rankTree::getRootPathLen(p->parent); }
  static localTree* getParent(localTree* r);
  static localTree* getRoot(localTree* r);
  static localTree* getLevelNode(localTree* r, size_t l);
  static nodeArr getRootPath(localTree* r);
  void insertToLeaf(size_t v, size_t l);
  void deleteEdge(size_t v, size_t l);
  size_t getEdgeLevel(size_t e);
  static void traverseTopDown(localTree* root, bool clear, bool verbose, bool stat, parlay::sequence<stats>& info);
  static std::tuple<bool, size_t, size_t> fetchEdge(localTree* root, size_t l);
  static localTree* getIfSingleton(localTree* r);
  static bool ifSingleton(localTree* r);
};
inline localTree::~localTree() {
  if (vertex) delete vertex;
}
inline localTree* localTree::mergeNode(localTree::nodeArr& Q, size_t l) {
  auto Cv = new localTree();
  Cv->size = 0;
  Cv->vertex = nullptr;
  Cv->parent = nullptr;
  Cv->edgemap = 0;
  Cv->level = l;
  for (size_t i = 0; i < Q.size(); i++) {
    Cv->edgemap = Cv->edgemap | Q[i]->edgemap;
    Cv->size += Q[i]->size;
    if (Q[i]->level > l) std::cout << Q[i]->level << ", " << l << std::endl;
    assert(Q[i]->level <= l);
    if (Q[i]->level < l || Q[i]->level == 0) {
      auto rTree = new rankTree(std::log2(Q[i]->size), Cv, Q[i], Q[i]->edgemap);
      Q[i]->parent = rTree;
      Cv->rTrees = rankTree::insertRankTree(Cv->rTrees, rTree, Cv);
    } else {
      Cv->rTrees = rankTree::Merge(Cv->rTrees, Q[i]->rTrees, Cv);
      delete Q[i];
    }
  }
  // Cv = getIfSingleton(Cv);
  assert(1 << Cv->level >= Cv->size);
  return Cv;
}
inline localTree::localTree(localTree* Cu, localTree* Cv) {
  assert(Cu->level >= Cv->level);
  this->level = Cu->level + 1;
  this->size = Cu->size + Cv->size;
  this->parent = nullptr;
  this->vertex = nullptr;
  this->edgemap = Cu->edgemap | Cv->edgemap;
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
    it->Node = r;
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
  p->edgemap = p->edgemap | son->edgemap;
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
  return p->Node;
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
  p->parent = nullptr;
}
inline size_t localTree::getEdgeLevel(size_t e) {
  return this->vertex->getLevel(e);
}
inline void localTree::traverseTopDown(localTree* root, bool clear, bool verbose, bool stat,
                                       parlay::sequence<stats>& info) {
  if (!root) return;
  stats::memUsage += sizeof(localTree);
  auto leaves = rankTree::decompose(root->rTrees, clear);
  if (stat) {
    // stats st(root->level, 0, 0, root->size);
    // if (!leaves.empty()) {
    //   st.fanout = leaves.size();
    //   st.height = root->rTrees[root->rTrees.size() - 1]->rank - leaves[0]->rank + 1;
    // }
    // info.push_back(std::move(st));
  }
  if (!leaves.empty()) fanout[leaves.size()]++;
  if (leaves.size() == 1)
    std::cout << root->level << " " << root->getSize() << " " << leaves[0]->descendant->level << " "
              << leaves[0]->descendant->getSize() << std::endl;
  for (auto it : leaves) {
    traverseTopDown(it->descendant, clear, verbose, stat, info);
    if (clear) delete it;
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