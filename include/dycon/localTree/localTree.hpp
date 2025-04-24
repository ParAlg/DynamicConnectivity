#pragma once
#include "alloc.hpp"
#include "dycon/localTree/graph.hpp"
#include "fetchQueue.hpp"
#include "leaf.hpp"
#include "parlay/parallel.h"
#include "rankTree.hpp"
#include "stats.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <utility>
class rankTree;
class localTree {
private:
  friend class rankTree;
  using nodeArr = std::vector<localTree *>;
  // Each time manipulating node, check if 6 members are covered.
  std::vector<rankTree *> rTrees;
  uint32_t level;
  uint32_t size; // number of vertices incident to this cluster node.
  rankTree *parent;
  leaf *vertex;
  std::bitset<64> edgemap;
  template <localTree *> static bool comp(localTree *x, localTree *y) {
    return x->size < y->size;
  }

public:
  static type_allocator<localTree> *l_alloc;
  localTree(uint32_t _id)
      : level(0), size(1), parent(nullptr), vertex(new leaf(_id, this)),
        edgemap(0) {
    rTrees.clear();
  }
  localTree()
      : level(0), size(0), parent(nullptr), vertex(nullptr), edgemap(0) {
    rTrees.clear();
  };
  localTree(localTree *Cu, localTree *Cv);
  ~localTree();
  void setBitMap(uint32_t l, bool val) { this->edgemap[l] = val; }
  uint32_t getLevel() { return this->level; }
  std::bitset<64> getMap() { return this->edgemap; }
  void setLevel(uint32_t l) {
    assert((1 << l) >= this->size);
    this->level = l;
  }
  void setSize(uint32_t sz) { this->size = sz; }
  uint32_t getSize() { return this->size; }
  static void merge(localTree *Cu, localTree *Cv);
  static void addChild(localTree *p, localTree *son);
  static void add2Children(localTree *p, localTree *s1, localTree *s2);
  static void addChildren(localTree *p, fetchQueue<localTree *> &nodes);
  static void deleteFromParent(localTree *son);
  static void deleteFromParent(localTree *p, fetchQueue<localTree *> &nodes);
  static localTree *splitFromParent(localTree *p,
                                    fetchQueue<localTree *> &nodes);
  static localTree *getParent(localTree *r);
  static localTree *getRoot(localTree *r);
  static localTree *getLevelNode(localTree *r, uint32_t l);
  static nodeArr getRootPath(localTree *r);
  void insertToLeaf(uint32_t v, uint32_t l);
  void deleteEdge(uint32_t v, uint32_t l);
  void addEdge(uint32_t v, uint32_t l);
  void changeLevel(std::vector<::vertex> &nghs, uint32_t oval, uint32_t nval);
  uint32_t getEdgeLevel(uint32_t e);
  static void updateBitMap(localTree *node);
  static void traverseTopDown(localTree *root, bool clear, bool verbose,
                              bool stat, parlay::sequence<stats> &info);
  static std::tuple<bool, uint32_t, uint32_t> fetchEdge(localTree *root,
                                                        uint32_t l);
  static std::optional<std::pair<::vertex, edge_set *>>
  fetchLeaf(localTree *root, uint32_t l);
  static bool ifSingleton(localTree *r);
  static edge_set *getEdgeSet(localTree *r, uint32_t l);
  static void topDown(localTree *root, bool clear,
                      std::atomic<size_t> &mem_usage);
  static size_t getLeafSpace(localTree *root);
};
inline type_allocator<localTree> *localTree::l_alloc = nullptr;
inline localTree::~localTree() {
  if (vertex)
    delete vertex;
}
inline localTree::localTree(localTree *Cu, localTree *Cv) {
  assert(Cu->level >= Cv->level);
  this->level = Cu->level + 1;
  this->size = Cu->size + Cv->size;
  this->parent = nullptr;
  this->vertex = nullptr;
  this->edgemap = Cu->edgemap | Cv->edgemap;
  assert((1 << Cu->level) >= Cu->size);
  assert((1 << Cv->level) >= Cv->size);
  auto ru =
      rankTree::r_alloc->create(std::log2(Cu->size), this, Cu, Cu->edgemap);
  auto rv =
      rankTree::r_alloc->create(std::log2(Cv->size), this, Cv, Cv->edgemap);
  Cu->parent = ru;
  Cv->parent = rv;
  this->rTrees = std::vector<rankTree *>({rv, ru});
}
inline bool localTree::ifSingleton(localTree *r) {
  if (!r)
    return false;
  if (r->rTrees.empty())
    return true;
  if (r->rTrees.size() == 1 && r->rTrees[0]->isleaf())
    return true;
  else
    return false;
}
inline void localTree::merge(localTree *Cu, localTree *Cv) {
  Cu->size = Cu->size + Cv->size;
  Cu->rTrees = rankTree::Merge(Cu->rTrees, Cv->rTrees, Cu);
  auto oval = Cu->edgemap;
  Cu->edgemap = Cu->edgemap | Cv->edgemap;
  assert(Cu->level == Cv->level);
  assert(1 << Cu->level >= Cu->size);
  l_alloc->free(Cv);
  if (Cu->edgemap != oval)
    updateBitMap(Cu);
}
inline void localTree::addChild(localTree *p, localTree *son) {
  if (!p)
    return;
  auto rTree =
      rankTree::r_alloc->create(std::log2(son->size), p, son, son->edgemap);
  son->parent = rTree;
  p->size += son->size;
  p->rTrees = rankTree::insertRankTree(p->rTrees, rTree, p);
  auto oval = p->edgemap;
  p->edgemap = p->edgemap | son->edgemap;
  if (p->edgemap != oval)
    updateBitMap(p);
  assert(p->level > son->level);
  assert((1 << p->level) >= p->size);
}
inline void localTree::add2Children(localTree *p, localTree *s1,
                                    localTree *s2) {
  if (!p)
    return;
  auto r1 = rankTree::r_alloc->create(std::log2(s1->size), p, s1, s1->edgemap);
  s1->parent = r1;
  p->rTrees = rankTree::insertRankTree(p->rTrees, r1, p);

  auto r2 = rankTree::r_alloc->create(std::log2(s2->size), p, s2, s2->edgemap);
  s2->parent = r2;
  p->rTrees = rankTree::insertRankTree(p->rTrees, r2, p);

  p->size += s1->size + s2->size;
  auto oval = p->edgemap;
  p->edgemap = p->edgemap | s1->edgemap | s2->edgemap;
  if (p->edgemap != oval)
    updateBitMap(p);

  assert(p->level > std::max(s1->level, s2->level));
  assert((1 << p->level) >= p->size);
}
inline void localTree::addChildren(localTree *p,
                                   fetchQueue<localTree *> &nodes) {
  // this one does not update bitmap to top because p has no parents
  for (auto it : nodes) {
    auto rTree =
        rankTree::r_alloc->create(std::log2(it->size), p, it, it->edgemap);
    it->parent = rTree;
    p->rTrees = rankTree::insertRankTree(p->rTrees, rTree, p);
    p->size += it->size;
  }
  p->edgemap = rankTree::getEdgeMap(p->rTrees);
}
inline localTree *localTree::getParent(localTree *r) {
  if (!r->parent)
    return nullptr;
  auto p = rankTree::getRoot(r->parent);
  return p->Node;
}
inline localTree *localTree::getRoot(localTree *r) {
  while (r->parent)
    r = getParent(r);
  return r;
}
// A level i edge connects level i-1 node Cu and Cv, return
inline localTree *localTree::getLevelNode(localTree *r, uint32_t l) {
  while (r->parent) {
    auto p = getParent(r);
    if (p->level == l)
      return r;
    r = p;
  }
  return nullptr;
}
inline localTree::nodeArr localTree::getRootPath(localTree *r) {
  localTree *temp[128];
  uint32_t num_nodes = 0;
  temp[num_nodes++] = r;
  while (r->parent) {
    r = getParent(r);
    temp[num_nodes++] = r;
  }
  nodeArr p(temp, temp + num_nodes);
  return p;
}
inline void localTree::updateBitMap(localTree *node) {
  while (node->parent) {
    node = rankTree::updateBitMapByTree(node->parent, node->edgemap);
    if (!node)
      break;
    auto oval = node->edgemap;
    std::bitset<64> nval = 0;
    for (auto it : node->rTrees)
      nval = nval | it->edgemap;
    if (nval == oval)
      break;
    node->edgemap = nval;
  }
}
inline void localTree::insertToLeaf(uint32_t v, uint32_t l) {
  assert(this->vertex != nullptr);
  this->vertex->insert(v, l);
  auto oval = this->edgemap;
  this->edgemap = this->vertex->getEdgeMap();
  if (this->edgemap != oval)
    updateBitMap(this);
}
inline void localTree::deleteEdge(uint32_t v, uint32_t l) {
  this->vertex->remove(v, l);
  auto oval = this->edgemap;
  this->edgemap = this->vertex->getEdgeMap();
  if (this->edgemap != oval)
    updateBitMap(this);
}
inline void localTree::deleteFromParent(localTree *son) {
  if (!son->parent)
    return;
  auto node = getParent(son);
  node->size -= son->size;
  auto oval = node->edgemap;
  node->rTrees = rankTree::remove(node->rTrees, son->parent, node);
  node->edgemap = rankTree::getEdgeMap(node->rTrees);
  if (node->edgemap != oval)
    updateBitMap(node);
  son->parent = nullptr;
}
inline void localTree::deleteFromParent(localTree *p,
                                        fetchQueue<localTree *> &nodes) {
  if (!p)
    return;
  std::vector<rankTree *> rTrees(nodes.size());
  uint32_t i = 0;
  uint32_t sz = 0;
  for (auto it : nodes) {
    assert(getParent(it) == p);
    rTrees[i++] = it->parent;
    it->parent = nullptr;
    sz += it->size;
  }
  auto oval = p->edgemap;

  p->rTrees = rankTree::remove(p->rTrees, rTrees, p);
  p->edgemap = rankTree::getEdgeMap(p->rTrees);
  if (p->edgemap != oval)
    updateBitMap(p);
  if (sz > p->size)
    std::abort();
  p->size -= sz;
}
inline localTree *localTree::splitFromParent(localTree *p,
                                             fetchQueue<localTree *> &nodes) {
  // delete nodes from p, all nodes have to be p's children and put these nodes
  // as C's children
  auto C = localTree::l_alloc->create();
  uint32_t _v = 0;
  uint32_t cl = 0;
  for (auto it : nodes) {
    _v += it->size;
    cl = std::max(cl, it->level);
  }
  C->level = std::max(cl, (uint32_t)std::ceil(std::log2(_v)));
  deleteFromParent(p, nodes);
  // assign nodes as C's children or sibling
  // this one does not update bitmap to top because C has no parents
  for (auto it : nodes) {
    if (it->getLevel() == C->getLevel()) {
      C->rTrees = rankTree::Merge(C->rTrees, it->rTrees, C);
      l_alloc->free(it);
    } else {
      auto rTree =
          rankTree::r_alloc->create(std::log2(it->size), C, it, it->edgemap);
      it->parent = rTree;
      C->rTrees = rankTree::insertRankTree(C->rTrees, rTree, C);
    }
  }
  C->size = _v;
  C->edgemap = rankTree::getEdgeMap(C->rTrees);
  return C;
}
inline uint32_t localTree::getEdgeLevel(uint32_t e) {
  return this->vertex->getLevel(e);
}
inline void localTree::traverseTopDown(localTree *root, bool clear,
                                       bool verbose, bool stat,
                                       parlay::sequence<stats> &info) {
  if (!root)
    return;
  stats::memUsage += sizeof(localTree);
  auto leaves = rankTree::decompose(root->rTrees, clear);
  if (stat) {
    stats st(root->level, 0, 0, root->size);
    if (!leaves.empty()) {
      st.fanout = leaves.size();
      st.height =
          root->rTrees[root->rTrees.size() - 1]->rank - leaves[0]->rank + 1;
    }
    info.push_back(std::move(st));
  }
  for (auto it : leaves) {
    traverseTopDown(it->descendant, clear, verbose, stat, info);
    if (clear)
      rankTree::r_alloc->free(it); // delete it;
  }
  if (clear)
    l_alloc->free(root); // delete root;
}
inline std::tuple<bool, uint32_t, uint32_t>
localTree::fetchEdge(localTree *root, uint32_t l) {
  if (root->edgemap[l] == 0)
    return std::make_tuple(false, 0, 0);
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
inline std::optional<std::pair<::vertex, edge_set *>>
localTree::fetchLeaf(localTree *root, uint32_t l) {
  if (root->level == 0) {
    assert(root->vertex != nullptr);
    return root->vertex->getLevelEdgeSet(l);
  }
  for (auto it : root->rTrees) {
    if (it->edgemap[l] == 1)
      return fetchLeaf(rankTree::fetchLeaf(it, l)->descendant, l);
  }
  return std::nullopt;
}
inline edge_set *localTree::getEdgeSet(localTree *r, uint32_t l) {
  assert(r->level == 0);
  return r->vertex->getLevelEdge(l);
}
inline void localTree::changeLevel(std::vector<::vertex> &nghs, uint32_t oval,
                                   uint32_t nval) {
  if (nghs.empty())
    return;
  auto omap = this->edgemap;
  // this->edgemap = this->vertex->changeLevel(nghs, oval, nval);
  if (this->edgemap != omap)
    localTree::updateBitMap(this);
}
inline void localTree::topDown(localTree *root, bool clear,
                               std::atomic<size_t> &mem_usage) {
  if (!root)
    return;
  mem_usage += sizeof(rankTree *) * root->rTrees.size();
  auto leaves = rankTree::decompose(root->rTrees, clear);
  parlay::parallel_for(0, leaves.size(), [&](auto i) {
    topDown(leaves[i]->descendant, clear, mem_usage);
  });
  if (clear)
    l_alloc->free(root);
}
inline size_t localTree::getLeafSpace(localTree *root) {
  if (!root || !root->vertex)
    return 0;
  return leaf::getLeafSpace(root->vertex);
}