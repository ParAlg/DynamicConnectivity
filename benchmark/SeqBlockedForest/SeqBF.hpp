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
  static localTree *pushEdge(localTree **pu, size_t uter, localTree **pv, size_t vter);
  void fixSatellite(localTree *CP, localTree **pu, size_t uter, localTree **pv, size_t vter);
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
  // cu cv have to be unblocked
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
inline void SeqBF::fixSatellite(localTree *CP, localTree **pu, size_t uter, localTree **pv, size_t vter) {
  auto Cu = pu[uter];
  auto Cv = pv[vter];
  auto puv = alloc();
  // let Q be the satellite, lf be the leaf
  auto Q = Cu->getSize() <= Cv->getSize() ? Cu : Cv;
  auto lf = Cu->getSize() <= Cv->getSize() ? pu[0] : pv[0];
  auto e = localTree::fetchEdge(Q, CP->getLevel());
  while (std::get<0>(e) == true) {
    size_t uvter = localTree::getNodePathLen(leaves[std::get<2>(e)], puv, CP->getLevel()) - 1;
    auto Cuv = puv[uvter];
    assert(Cuv != nullptr);
    assert(CP == nullptr || localTree::getParent(Cuv) == CP);
    if (Cuv != Q) {
      // replacement edge found
      if (Cuv->getSize() + Q->getSize() > 1 << std::max(Cuv->getLevel(), Q->getLevel())) {
        if (CP->getLevel() - std::max(Cuv->getLevel(), Q->getLevel()) == 1) {
          // blocked edge in this level don't push
          // satellite to center
        } else {
          // unblocked edge needs to compress
          // satellite to satellite no pushdown
          leaves[std::get<1>(e)]->deleteEdge(std::get<2>(e), CP->getLevel());
          leaves[std::get<2>(e)]->deleteEdge(std::get<1>(e), CP->getLevel());
          localTree::deleteFromParent(Cuv);
          localTree::deleteFromParent(Q);
          auto np = Cuv->getLevel() >= Q->getLevel() ? new localTree(Cuv, Q) : new localTree(Q, Cuv);
          localTree::addChild(CP, np);
          leaves[std::get<1>(e)]->insertToLeaf(std::get<2>(e), np->getLevel());
          leaves[std::get<2>(e)]->insertToLeaf(std::get<1>(e), np->getLevel());
        }
        assert(CP->getMap()[CP->getLevel()] ==
               true);  // satellite is incident to a blocked edge so parent shouldn't be singleton
      } else {
        // satellite to satellite push down
        leaves[std::get<1>(e)]->deleteEdge(std::get<2>(e), CP->getLevel());
        leaves[std::get<2>(e)]->deleteEdge(std::get<1>(e), CP->getLevel());
        auto pq = alloc();
        size_t qter = localTree::getNodePathLen(lf, pq, CP->getLevel()) - 1;
        auto np = pushDown(puv, uvter, pq, qter);
        dealloc(pq);
        leaves[std::get<1>(e)]->insertToLeaf(std::get<2>(e), np->getLevel());
        leaves[std::get<2>(e)]->insertToLeaf(std::get<1>(e), np->getLevel());
      }
      dealloc(puv);
      return;
    } else {
      // self loop
      // find lca
      // push down
      leaves[std::get<1>(e)]->deleteEdge(std::get<2>(e), CP->getLevel());
      leaves[std::get<2>(e)]->deleteEdge(std::get<1>(e), CP->getLevel());
      auto pq = alloc();
      size_t qter = localTree::getNodePathLen(lf, pq, CP->getLevel()) - 1;
      auto lca = Q;
      while (pq[qter] == puv[uvter]) {
        lca = pq[qter];
        qter--;
        uvter--;
      }
      localTree *np = nullptr;
      if (pq[qter]->getSize() + puv[uvter]->getSize() <= 1 << std::max(pq[qter]->getLevel(), puv[uvter]->getLevel()))
        np = pushDown(pq, qter, puv, uvter);
      else {
        if (lca->getLevel() - std::max(pq[qter]->getLevel(), puv[uvter]->getLevel()) == 1)
          np = lca;
        else {
          localTree::deleteFromParent(pq[qter]);
          localTree::deleteFromParent(puv[uvter]);
          np = pq[qter]->getLevel() > puv[uvter]->getLevel() ? new localTree(pq[qter], puv[uvter])
                                                             : new localTree(puv[uvter], pq[qter]);
          localTree::addChild(lca, np);
        }
      }
      leaves[std::get<1>(e)]->insertToLeaf(std::get<2>(e), np->getLevel());
      leaves[std::get<2>(e)]->insertToLeaf(std::get<1>(e), np->getLevel());
      dealloc(pq);
      return;
    }
    e = localTree::fetchEdge(Q, CP->getLevel());
  }
  auto GP = localTree::getParent(CP);

  localTree::deleteFromParent(CP);
  localTree::deleteFromParent(Q);
  if (CP->getMap()[CP->getLevel()] == false) CP = localTree::getIfSingleton(CP);
  if (!GP) return;

  if (GP->getMaxChild()->getSize() < CP->getSize() + Q->getSize()) {
    // fix star center
    uter = localTree::getNodePathLen(pu[0], pu, GP->getLevel()) - 1;
    vter = localTree::getNodePathLen(pv[0], pv, GP->getLevel()) - 1;
    fixSatellite(GP, pu, uter, pv, vter);
  } else {
    // fix satellite
    localTree::addChild(GP, Q);
    localTree::addChild(GP, CP);
    uter = localTree::getNodePathLen(pu[0], pu, GP->getLevel()) - 1;
    vter = localTree::getNodePathLen(pv[0], pv, GP->getLevel()) - 1;
    fixSatellite(GP, pu, uter, pv, vter);
  }
}
inline void SeqBF::remove(size_t u, size_t v) {
  assert(leaves[u]->getEdgeLevel(v) == leaves[v]->getEdgeLevel(u));
  size_t l = leaves[u]->getEdgeLevel(v);
  leaves[u]->deleteEdge(v, l);
  leaves[v]->deleteEdge(u, l);
  auto pu = alloc();
  auto pv = alloc();
  size_t uLen = localTree::getNodePathLen(leaves[u], pu, l);
  size_t vLen = localTree::getNodePathLen(leaves[v], pv, l);
  assert(pu[uLen - 1] != nullptr && pv[vLen - 1] != nullptr);
  if (pu[uLen - 1] == pv[vLen - 1]) return;
  assert(localTree::getParent(pu[uLen - 1]) == localTree::getParent(pv[vLen - 1]));
  if (pu[uLen - 1]->getSize() + pv[vLen - 1]->getSize() <= 1 << l) return;  // edges between satellites.
  fixSatellite(localTree::getParent(pu[uLen - 1]), pu, uLen - 1, pv, vLen - 1);
}
inline void SeqBF::run_stat(std::string filepath, bool verbose = false, bool clear = false, bool stat = true) {
  return;
}