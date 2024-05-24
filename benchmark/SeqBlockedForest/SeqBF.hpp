#include "localTree.hpp"
class SeqBF {
 private:
  struct pathblock {
    char x[sizeof(localTree *) * 65];
  };
  using pathAllocator = parlay::type_allocator<pathblock>;
  static localTree **alloc() { return (localTree **)pathAllocator::alloc(); }
  static void dealloc(localTree **p) { pathAllocator::free((pathblock *)p); }
  static localTree *pushDown(localTree **pu, size_t uter, localTree **pv, size_t vter);
  void placeEdges(parlay::sequence<std::pair<size_t, size_t>> &edges, size_t l);
  void printNodes(parlay::sequence<localTree *> &Nodes) {
    std::copy(Nodes.begin(), Nodes.end(), std::ostream_iterator<localTree *>(std::cout, ","));
    std::cout << "\n\n";
  }

 public:
  size_t n;
  static size_t lmax;
  parlay::sequence<localTree *> leaves;
  SeqBF(size_t _n) : n(_n), leaves(parlay::sequence<localTree *>(n, nullptr)) {}
  ~SeqBF() { run_stat("./", false, true, false); };
  void insert(size_t u, size_t v);
  bool is_connected(size_t u, size_t v);
  void remove(size_t u, size_t v);
  void run_stat(std::string filepath, bool verbose, bool clear, bool stat);
};
inline size_t SeqBF::lmax = 63;
inline void SeqBF::insert(size_t u, size_t v) {
  auto pu = alloc();
  auto pv = alloc();
  auto g = [&](size_t x, localTree **p) -> size_t {
    if (leaves[x] == nullptr) leaves[x] = new localTree(x);
    return localTree::getRootPathLen(leaves[x], p);
  };
  size_t uLen = g(u, pu);
  size_t vLen = g(v, pv);
  localTree *rootu = pu[uLen - 1];
  localTree *rootv = pv[vLen - 1];
  localTree *np = nullptr;
  if (rootu != rootv) {
    if (rootu->getSize() + rootv->getSize() > (1 << std::max(rootu->getLevel(), rootv->getLevel())))
      np = rootu->getLevel() >= rootv->getLevel() ? new localTree(rootu, rootv) : new localTree(rootv, rootu);
    else {
      if (rootu->getLevel() == rootv->getLevel()) {
        localTree::merge(rootu, rootv);
        np = rootu->getLevel() > 1 ? pushDown(pu, uLen - 2, pv, vLen - 2) : rootu;
      } else {
        if (rootu->getLevel() > rootv->getLevel()) {
          localTree::addChild(rootu, rootv);
          np = pushDown(pu, uLen - 2, pv, vLen - 1);
        } else {
          localTree::addChild(rootv, rootu);
          np = pushDown(pu, uLen - 1, pv, vLen - 2);
        }
      }
    }
  } else {
    localTree *lca = nullptr;
    while (pu[uLen - 1] == pv[vLen - 1]) {
      lca = pu[uLen - 1];
      uLen--;
      vLen--;
    }
    if (pu[uLen - 1]->getSize() + pv[vLen - 1]->getSize() >
        1 << std::max(pu[uLen - 1]->getLevel(), pv[vLen - 1]->getLevel())) {
      if (lca->getLevel() - std::max(pu[uLen - 1]->getLevel(), pv[vLen - 1]->getLevel()) > 1) {
        localTree::deleteFromParent(pu[uLen - 1]);
        localTree::deleteFromParent(pv[vLen - 1]);
        np = pu[uLen - 1]->getLevel() >= pv[vLen - 1]->getLevel() ? new localTree(pu[uLen - 1], pv[vLen - 1])
                                                                  : new localTree(pv[vLen - 1], pu[uLen - 1]);
        localTree::addChild(lca, np);
      } else
        np = lca;
    } else
      np = pushDown(pu, uLen - 1, pv, vLen - 1);
  }
  leaves[u]->insertToLeaf(v, np->getLevel());
  leaves[v]->insertToLeaf(u, np->getLevel());
  dealloc(pu);
  dealloc(pv);
}
inline localTree *SeqBF::pushDown(localTree **pu, size_t uter, localTree **pv, size_t vter) {
  while (pu[uter]->getSize() + pv[vter]->getSize() <= 1 << std::max(pu[uter]->getLevel(), pv[vter]->getLevel())) {
    if (pu[uter]->getLevel() < pv[vter]->getLevel()) {
      std::swap(uter, vter);
      std::swap(pu, pv);
    }
    auto p = localTree::getParent(pu[uter]);
    if (pu[uter]->getLevel() == pv[vter]->getLevel()) {
      localTree::deleteFromParent(pu[uter]);
      localTree::deleteFromParent(pv[vter]);
      localTree::merge(pu[uter], pv[vter]);
      localTree::addChild(p, pu[uter]);
      uter--;
      vter--;
    } else if (p->getLevel() - pu[uter]->getLevel() == 1) {
      localTree::deleteFromParent(pu[uter]);
      localTree::deleteFromParent(pv[vter]);
      localTree::addChild(pu[uter], pv[vter]);
      localTree::addChild(p, pu[uter]);
      uter--;
    } else {
      localTree::deleteFromParent(pu[uter]);
      localTree::deleteFromParent(pv[vter]);
      auto np = new localTree(pu[uter], pv[vter]);
      localTree::addChild(p, np);
      return np;
    }
  }
  assert(localTree::getParent(pu[uter]) == localTree::getParent(pv[vter]));
  return localTree::getParent(pu[uter]);
}
inline void SeqBF::run_stat(std::string filepath, bool verbose = false, bool clear = false, bool stat = true) {
  return;
}
inline void SeqBF::remove(size_t u, size_t v) {
  assert(leaves[u]->getEdgeLevel(v) == leaves[v]->getEdgeLevel(u));
  size_t l = leaves[u]->getEdgeLevel(v);
  leaves[u]->deleteEdge(v, l);
  leaves[v]->deleteEdge(u, l);
  auto Cu = localTree::getLevelNode(leaves[u], l);
  auto Cv = localTree::getLevelNode(leaves[v], l);
  assert(Cu != nullptr && Cv != nullptr);
  auto CP = localTree::getParent(Cu);
  if (Cu == Cv) return;
  assert(localTree::getParent(Cu) == localTree::getParent(Cv));
}