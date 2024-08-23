#include "localTree.hpp"
#include "parlay/sequence.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <queue>
#include <unordered_set>
class SCCWN {
private:
  static std::tuple<bool, size_t, size_t> fetchEdge(std::queue<localTree *> &Q,
                                                    size_t l);
  static localTree *pushDown(parlay::sequence<localTree *> &pu, size_t uter,
                             parlay::sequence<localTree *> &pv, size_t vter);
  void placeEdges(parlay::sequence<std::pair<size_t, size_t>> &edges, size_t l);
  void printNodes(parlay::sequence<localTree *> &Nodes) {
    std::copy(Nodes.begin(), Nodes.end(),
              std::ostream_iterator<localTree *>(std::cout, ","));
    std::cout << "\n\n";
  }

public:
  size_t n;
  static size_t lmax;
  parlay::sequence<localTree *> leaves;
  SCCWN(size_t _n) : n(_n), leaves(parlay::sequence<localTree *>(n, nullptr)) {
    parlay::internal::timer t;
    leaves = parlay::sequence<localTree *>(n);
    for (uint32_t i = 0; i < n; i++)
      leaves[i] = new localTree(i);
    t.next("constructor");
  }
  ~SCCWN() { run_stat("./", false, true, false); };
  void insertToLCA(size_t u, size_t v);
  void insertToRoot(size_t u, size_t v);
  void insertToBlock(size_t u, size_t v);
  bool is_connected(size_t u, size_t v);
  void remove(size_t u, size_t v);
  void run_stat(std::string filepath, bool verbose, bool clear, bool stat);
  void checkLevel() {
    for (size_t i = 0; i < n; i++) {
      if (leaves[i] != nullptr) {
        auto r = leaves[i];
        while (localTree::getParent(r) != nullptr) {
          if (localTree::getParent(r)->getLevel() <= r->getLevel()) {
            std::cout << "parent level = "
                      << localTree::getParent(r)->getLevel()
                      << " parent size = " << localTree::getParent(r)->getSize()
                      << " child level = " << r->getLevel()
                      << " child size = " << r->getSize() << std::endl;
            exit(0);
          }
          r = localTree::getParent(r);
        }
      }
    }
  }
};
inline size_t SCCWN::lmax = 63;
inline void SCCWN::insertToRoot(size_t u, size_t v) {
  auto g = [&](size_t &u) -> localTree * {
    localTree *r = localTree::getRoot(leaves[u]);
    if (r->getLevel() == lmax)
      return r;
    auto p = new localTree();
    p->setLevel(lmax);
    localTree::addChild(p, r);
    return p;
  };
  if (leaves[u] == nullptr)
    leaves[u] = new localTree(u);
  if (leaves[v] == nullptr)
    leaves[v] = new localTree(v);
  auto Cu = g(u);
  auto Cv = g(v);
  if (Cu != Cv)
    localTree::merge(Cu, Cv);
  leaves[u]->insertToLeaf(v, lmax);
  leaves[v]->insertToLeaf(u, lmax);
}
inline void SCCWN::insertToLCA(size_t u, size_t v) {
  if (leaves[u] == nullptr)
    leaves[u] = new localTree(u);
  if (leaves[v] == nullptr)
    leaves[v] = new localTree(v);
  auto Cu = leaves[u];
  auto Cv = leaves[v];
  localTree *Pu = nullptr, *Pv = nullptr;
  // if we find LCA in this while loop, then we return
  // after the while loop, Cu and Cv will be the two roots respectively
  while (true) {
    while (Cu->getLevel() == Cv->getLevel()) {
      // if Cu Cv in same level, we traverse to their parents
      // if one of two do not have parent, we go up to roots for both
      // because they cannot be in the same CF
      if (Cu == Cv) { // LCA
        auto P = localTree::getParent(Cu);
        size_t l = (P == nullptr) ? Cu->getLevel() : P->getLevel();
        leaves[u]->insertToLeaf(v, l);
        leaves[v]->insertToLeaf(u, l);
        return;
      }
      Pu = localTree::getParent(Cu);
      Pv = localTree::getParent(Cv);
      if (Pu == nullptr) {
        while (Pv != nullptr) {
          Cv = Pv;
          Pv = localTree::getParent(Cv);
        }
        break;
      }
      if (Pv == nullptr) {
        while (Pu != nullptr) {
          Cu = Pu;
          Pu = localTree::getParent(Cu);
        }
        break;
      }
      Cu = Pu;
      Cv = Pv;
    }
    if (Pu == nullptr && Pv == nullptr)
      break;
    if (Cu->getLevel() > Cv->getLevel())
      std::swap(Cu, Cv);
    while (Cu->getLevel() < Cv->getLevel()) {
      Pu = localTree::getParent(Cu);
      if (Pu == nullptr) {
        Pv = localTree::getParent(Cv);
        while (Pv != nullptr) {
          Cv = Pv;
          Pv = localTree::getParent(Pv);
        }
        break;
      }
      Cu = Pu;
    }
    if (Pu == nullptr && Pv == nullptr)
      break;
    assert(Cu->getLevel() >= Cv->getLevel());
  }
  assert(Cu != nullptr && Cv != nullptr);
  assert(localTree::getParent(Cu) == nullptr);
  assert(localTree::getParent(Cv) == nullptr);
  assert(Cu != Cv);
  size_t l;
  if (Cu->getLevel() < Cv->getLevel())
    std::swap(Cu, Cv); // make sure left is higher
  if (Cu->getSize() + Cv->getSize() <= (1 << Cu->getLevel())) {
    if (Cu->getLevel() == Cv->getLevel()) {
      localTree::merge(Cu, Cv);
      l = Cu->getLevel();
    } else {
      localTree::addChild(Cu, Cv);
      l = Cu->getLevel();
    }
  } else {
    auto r = new localTree(Cu, Cv);
    l = r->getLevel();
  }
  leaves[u]->insertToLeaf(v, l);
  leaves[v]->insertToLeaf(u, l);
}
// inline void SCCWN::insertToLCA(size_t u, size_t v) {
//   if (leaves[u] == nullptr)
//     leaves[u] = new localTree(u);
//   if (leaves[v] == nullptr)
//     leaves[v] = new localTree(v);
// auto pv = localTree::getRootPath(leaves[v]);
// auto pu = localTree::getRootPath(leaves[u]);
// auto iu = pu.rbegin();
// auto iv = pv.rbegin();
// size_t l;
// if (*iu != *iv) {
//   auto Cu = *iu;
//   auto Cv = *iv;
//   if (Cu->getLevel() < Cv->getLevel())
//     std::swap(Cu, Cv); // make sure left is higher
//   if (Cu->getSize() + Cv->getSize() <= (1 << Cu->getLevel())) {
//     if (Cu->getLevel() == Cv->getLevel()) {
//       localTree::merge(Cu, Cv);
//       l = Cu->getLevel();
//     } else {
//       localTree::addChild(Cu, Cv);
//       l = Cu->getLevel();
//     }
//   } else {
//     auto r = new localTree(Cu, Cv);
//     l = r->getLevel();
//   }
// } else {
//   localTree *p;
//   while (*iu == *iv) {
//     p = *iu;
//     iu++;
//     iv++;
//   }
//   localTree *gp = localTree::getParent(p);
//   l = gp == nullptr ? p->getLevel() : gp->getLevel();
// }
// leaves[u]->insertToLeaf(v, l);
// leaves[v]->insertToLeaf(u, l);
// }

// allocator
// modify rank tree
inline void SCCWN::insertToBlock(size_t u, size_t v) {
  if (leaves[u] == nullptr)
    leaves[u] = new localTree(u);
  if (leaves[v] == nullptr)
    leaves[v] = new localTree(v);
  auto pv = localTree::getRootPath(leaves[v]);
  auto pu = localTree::getRootPath(leaves[u]);
  size_t uLen = pu.size();
  size_t vLen = pv.size();
  localTree *rootu = pu[uLen - 1];
  localTree *rootv = pv[vLen - 1];
  localTree *np = nullptr;
  if (rootu != rootv) {
    if (rootu->getSize() + rootv->getSize() >
        (1 << std::max(rootu->getLevel(), rootv->getLevel())))
      np = rootu->getLevel() >= rootv->getLevel() ? new localTree(rootu, rootv)
                                                  : new localTree(rootv, rootu);
    else {
      if (rootu->getLevel() == rootv->getLevel()) {
        localTree::merge(rootu, rootv);
        np = rootu->getLevel() > 1 ? pushDown(pu, uLen - 2, pv, vLen - 2)
                                   : rootu;
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
      if (lca->getLevel() -
              std::max(pu[uLen - 1]->getLevel(), pv[vLen - 1]->getLevel()) >
          1) {
        localTree::deleteFromParent(pu[uLen - 1]);
        localTree::deleteFromParent(pv[vLen - 1]);
        np = pu[uLen - 1]->getLevel() >= pv[vLen - 1]->getLevel()
                 ? new localTree(pu[uLen - 1], pv[vLen - 1])
                 : new localTree(pv[vLen - 1], pu[uLen - 1]);
        localTree::addChild(lca, np);
      } else
        np = lca;
    } else
      np = pushDown(pu, uLen - 1, pv, vLen - 1);
  }
  leaves[u]->insertToLeaf(v, np->getLevel());
  leaves[v]->insertToLeaf(u, np->getLevel());
}
inline localTree *SCCWN::pushDown(parlay::sequence<localTree *> &pu,
                                  size_t uter,
                                  parlay::sequence<localTree *> &pv,
                                  size_t vter) {
  while (pu[uter]->getSize() + pv[vter]->getSize() <=
         1 << std::max(pu[uter]->getLevel(), pv[vter]->getLevel())) {
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
    } else {
      localTree::deleteFromParent(pu[uter]);
      localTree::deleteFromParent(pv[vter]);
      localTree::addChild(pu[uter], pv[vter]);
      localTree::addChild(p, pu[uter]);
      uter--;
      // } else if (p->getLevel() - pu[uter]->getLevel() == 1) {
      //   localTree::deleteFromParent(pu[uter]);
      //   localTree::deleteFromParent(pv[vter]);
      //   localTree::addChild(pu[uter], pv[vter]);
      //   localTree::addChild(p, pu[uter]);
      //   uter--;
      // } else {
      //   localTree::deleteFromParent(pu[uter]);
      //   localTree::deleteFromParent(pv[vter]);
      //   auto np = new localTree(pu[uter], pv[vter]);
      //   localTree::addChild(p, np);
      //   return np;
    }
  }
  assert(localTree::getParent(pu[uter]) == localTree::getParent(pv[vter]));
  return localTree::getParent(pu[uter]);
}
inline void SCCWN::remove(size_t u, size_t v) {
  // std::cout << u << " " << v << std::endl;
  assert(leaves[u]->getEdgeLevel(v) == leaves[v]->getEdgeLevel(u));
  size_t l = leaves[u]->getSize() < leaves[v]->getSize()
                 ? leaves[u]->getEdgeLevel(v)
                 : leaves[v]->getEdgeLevel(u);
  leaves[u]->deleteEdge(v, l);
  leaves[v]->deleteEdge(u, l);
  auto Cu = localTree::getLevelNode(leaves[u], l);
  auto Cv = localTree::getLevelNode(leaves[v], l);
  assert(Cu != nullptr && Cv != nullptr);
  auto CP = localTree::getParent(Cu);
  if (Cu == Cv)
    return;
  assert(localTree::getParent(Cu) == localTree::getParent(Cv));

  std::queue<localTree *> Qu, Qv;                     // ready to fetch
  parlay::sequence<std::pair<size_t, size_t>> Eu, Ev; // fetched edge
  std::unordered_set<localTree *> Hu, Hv;             // visited node
  parlay::sequence<localTree *> Ru, Rv;               // visited node
  auto init = [](std::queue<localTree *> &Q,
                 parlay::sequence<std::pair<size_t, size_t>> &E,
                 parlay::sequence<localTree *> &R,
                 std::unordered_set<localTree *> &HT, localTree *C) -> void {
    Q = std::queue<localTree *>();
    E = parlay::sequence<std::pair<size_t, size_t>>();
    HT = std::unordered_set<localTree *>();
    R = parlay::sequence<localTree *>();
    Q.push(C);
    HT.insert(C);
    R.push_back(C);
    E.clear();
  };
  while (l != 0) {
    init(Qu, Eu, Ru, Hu, Cu);
    init(Qv, Ev, Rv, Hv, Cv);
    auto nCu = Cu->getSize();
    auto nCv = Cv->getSize();
    assert(Cu != Cv);
    while (true) {
      auto eu = fetchEdge(Qu, l);
      if (std::get<0>(eu) == true) {
        auto Cuv = localTree::getLevelNode(leaves[std::get<2>(eu)], l);
        assert(Cuv != nullptr);
        assert(CP == nullptr || localTree::getParent(Cuv) == CP);
        if (Hv.find(Cuv) != Hv.end()) {
          if (Eu.empty()) {
            // don't push nCu+nCv may violate size
            return;
          } else if (nCu <= nCv) {
            for (auto it : Ru)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            size_t _v = 0;
            for (auto it : Ru) {
              _v += it->getSize();
              if (it->getLevel() > C->getLevel())
                C->setLevel(it->getLevel());
            }
            C->setLevel(
                std::max(C->getLevel(), (size_t)std::ceil(std::log2(_v))));
            for (auto it : Ru) {
              if (it->getLevel() == C->getLevel())
                localTree::merge(C, it);
              else
                localTree::addChild(C, it);
            }
            localTree::addChild(CP, C);
            placeEdges(Eu, C->getLevel());
            placeEdges(Ev, l);
          } else {
            for (auto it : Rv)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            size_t _v = 0;
            for (auto it : Rv) {
              _v += it->getSize();
              if (it->getLevel() > C->getLevel())
                C->setLevel(it->getLevel());
            }
            C->setLevel(
                std::max(C->getLevel(), (size_t)std::ceil(std::log2(_v))));
            for (auto it : Rv) {
              if (it->getLevel() == C->getLevel())
                localTree::merge(C, it);
              else
                localTree::addChild(C, it);
            }
            localTree::addChild(CP, C);
            placeEdges(Eu, l);
            placeEdges(Ev, C->getLevel());
          }
          // Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(),
          // nCv, l); Rstat.push_back(std::move(info));
          return;
        } else {
          if (Hu.find(Cuv) == Hu.end()) {
            Hu.insert(Cuv);
            Ru.push_back(Cuv);
            Qu.push(Cuv);
            nCu += Cuv->getSize();
          }
          Eu.push_back(std::make_pair(std::get<1>(eu), std::get<2>(eu)));
          leaves[std::get<1>(eu)]->deleteEdge(std::get<2>(eu), l);
          leaves[std::get<2>(eu)]->deleteEdge(std::get<1>(eu), l);
        }
      } else {
        auto GP = localTree::getParent(CP);
        localTree::deleteFromParent(CP);
        localTree *_CP;
        if (Eu.empty()) {
          localTree::deleteFromParent(Ru[0]);
          _CP = Ru[0];
          if (localTree::ifSingleton(CP) == true && CP->getMap()[l] == false) {
            localTree::deleteFromParent(Rv[0]);
            delete CP;
            CP = Rv[0];
          }
          // localTree::addChild(_CP, Ru[0]);
        } else if (nCu <= nCv) {
          for (auto it : Ru)
            localTree::deleteFromParent(it);
          _CP = new localTree();
          size_t _v = 0;
          for (auto it : Ru) {
            _v += it->getSize();
            if (it->getLevel() > _CP->getLevel())
              _CP->setLevel(it->getLevel());
          }
          _CP->setLevel(
              std::max(_CP->getLevel(), (size_t)std::ceil(std::log2(_v))));
          for (auto it : Ru) {
            if (it->getLevel() == _CP->getLevel())
              localTree::merge(_CP, it);
            else
              localTree::addChild(_CP, it);
          }
          placeEdges(Eu, _CP->getLevel());
          placeEdges(Ev, l);
        } else {
          _CP = new localTree();
          _CP->setLevel(l);
          for (auto it : Ru)
            localTree::deleteFromParent(it);
          for (auto it : Ru)
            localTree::addChild(_CP, it);
          auto oldMap = CP->getMap();
          for (auto it : Rv)
            localTree::deleteFromParent(it);
          auto C = new localTree();
          size_t _v = 0;
          for (auto it : Rv) {
            _v += it->getSize();
            if (it->getLevel() > C->getLevel())
              C->setLevel(it->getLevel());
          }
          C->setLevel(
              std::max(C->getLevel(), (size_t)std::ceil(std::log2(_v))));
          for (auto it : Rv) {
            if (it->getLevel() == C->getLevel())
              localTree::merge(C, it);
            else
              localTree::addChild(C, it);
          }
          if (oldMap[l] == false) {
            delete CP;
            CP = C;
          } else
            localTree::addChild(CP, C);
          placeEdges(Eu, l);
          placeEdges(Ev, C->getLevel());
        }
        localTree::add2Children(GP, CP, _CP);
        Cu = _CP;
        Cv = CP;
        CP = GP;
        l = CP ? CP->getLevel() : 0;

        // Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(),
        // nCv, l); Rstat.push_back(std::move(info));

        break;
      }
      auto ev = fetchEdge(Qv, l);
      if (std::get<0>(ev) == true) {
        auto Cuv = localTree::getLevelNode(leaves[std::get<2>(ev)], l);
        assert(Cuv != nullptr);
        assert(CP == nullptr || localTree::getParent(Cuv) == CP);
        if (Hu.find(Cuv) != Hu.end()) {
          if (Ev.empty()) {
            placeEdges(Eu, l);
          } else if (nCu <= nCv) {
            for (auto it : Ru)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            size_t _v = 0;
            for (auto it : Ru) {
              _v += it->getSize();
              if (it->getLevel() > C->getLevel())
                C->setLevel(it->getLevel());
            }
            C->setLevel(
                std::max(C->getLevel(), (size_t)std::ceil(std::log2(_v))));
            for (auto it : Ru) {
              if (it->getLevel() == C->getLevel())
                localTree::merge(C, it);
              else
                localTree::addChild(C, it);
            }
            localTree::addChild(CP, C);
            placeEdges(Eu, C->getLevel());
            placeEdges(Ev, l);
          } else {
            for (auto it : Rv)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            size_t _v = 0;
            for (auto it : Rv) {
              _v += it->getSize();
              if (it->getLevel() > C->getLevel())
                C->setLevel(it->getLevel());
            }
            C->setLevel(
                std::max(C->getLevel(), (size_t)std::ceil(std::log2(_v))));
            for (auto it : Rv) {
              if (it->getLevel() == C->getLevel())
                localTree::merge(C, it);
              else
                localTree::addChild(C, it);
            }
            localTree::addChild(CP, C);
            placeEdges(Eu, l);
            placeEdges(Ev, C->getLevel());
          }

          // Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(),
          // nCv, l); Rstat.push_back(std::move(info));
          return;
        } else {
          if (Hv.find(Cuv) == Hv.end()) {
            Hv.insert(Cuv);
            Rv.push_back(Cuv);
            Qv.push(Cuv);
            nCv += Cuv->getSize();
          }
          Ev.push_back(std::make_pair(std::get<1>(ev), std::get<2>(ev)));
          leaves[std::get<1>(ev)]->deleteEdge(std::get<2>(ev), l);
          leaves[std::get<2>(ev)]->deleteEdge(std::get<1>(ev), l);
        }
      } else {
        auto GP = localTree::getParent(CP);
        localTree::deleteFromParent(CP);
        localTree *_CP; // = new localTree();
        if (Ev.empty()) {
          localTree::deleteFromParent(Rv[0]);
          // localTree::addChild(_CP, Rv[0]);
          _CP = Rv[0];
          placeEdges(Eu, l);
        } else if (nCv <= nCu) {
          for (auto it : Rv)
            localTree::deleteFromParent(it);
          _CP = new localTree();
          size_t _v = 0;
          for (auto it : Rv) {
            _v += it->getSize();
            if (it->getLevel() > _CP->getLevel())
              _CP->setLevel(it->getLevel());
          }
          _CP->setLevel(
              std::max(_CP->getLevel(), (size_t)std::ceil(std::log2(_v))));
          for (auto it : Rv) {
            if (it->getLevel() == _CP->getLevel())
              localTree::merge(_CP, it);
            else
              localTree::addChild(_CP, it);
          }
          placeEdges(Eu, l);
          placeEdges(Ev, _CP->getLevel());
        } else {
          _CP = new localTree();
          _CP->setLevel(l);
          for (auto it : Rv)
            localTree::deleteFromParent(it);
          for (auto it : Rv)
            localTree::addChild(_CP, it);
          auto oldMap = CP->getMap();
          for (auto it : Ru)
            localTree::deleteFromParent(it);
          auto C = new localTree();
          size_t _v = 0;
          for (auto it : Ru) {
            _v += it->getSize();
            if (it->getLevel() > C->getLevel())
              C->setLevel(it->getLevel());
          }
          C->setLevel(
              std::max(C->getLevel(), (size_t)std::ceil(std::log2(_v))));
          for (auto it : Ru) {
            if (it->getLevel() == C->getLevel())
              localTree::merge(C, it);
            else
              localTree::addChild(C, it);
          }
          if (oldMap[l] == false) {
            delete CP;
            CP = C;
          } else
            localTree::addChild(CP, C);
          placeEdges(Eu, C->getLevel());
          placeEdges(Ev, l);
        }
        localTree::add2Children(GP, CP, _CP);
        Cu = CP;
        Cv = _CP;
        CP = GP;
        l = CP ? CP->getLevel() : 0;

        // Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(),
        // nCv, l); Rstat.push_back(std::move(info));

        break;
      }
    }
  }
}
inline bool SCCWN::is_connected(size_t u, size_t v) {
  return localTree::getRoot(leaves[u]) == localTree::getRoot(leaves[v]);
}

inline void SCCWN::run_stat(std::string filepath, bool verbose = false,
                            bool clear = false, bool stat = true) {
  parlay::sequence<localTree *> roots(leaves.size(), nullptr);
  parlay::sequence<localTree *> parents(leaves.size(), nullptr);
  parlay::parallel_for(0, roots.size(), [&](size_t i) {
    if (leaves[i])
      roots[i] = localTree::getRoot(leaves[i]);
    if (leaves[i])
      parents[i] = localTree::getParent(leaves[i]);
  });
  stats::memUsage = 0;
  if (verbose)
    printNodes(leaves);
  if (verbose)
    printNodes(parents);
  if (verbose)
    printNodes(roots);
  roots = parlay::remove_duplicates(roots);
  if (verbose)
    printNodes(roots);
  parlay::parallel_for(0, roots.size(), [&](size_t i) {
    if (roots[i]) {
      // std::ofstream fout;
      // if (stat) fout.open(filepath + "/" + std::to_string(i) + ".txt");
      parlay::sequence<stats> info;
      localTree::traverseTopDown(roots[i], clear, verbose, stat, info);
      // if (stat) {
      //   parlay::sort_inplace(info, [&](stats x, stats y) { return x.level >
      //   y.level; }); for (auto it : info)
      //     fout << it.level << " " << it.fanout << " " << it.height << " "
      //     << it.size << std::endl;
      //   fout.close();
      // }
    }
  });
  // if (stat) std::cout << "quiet memory usage is " << stats::memUsage << "
  // bytes\n"; if (stat) {
  //   parlay::sort_inplace(Rstat, [&](const Radius &a, const Radius &b) {
  //   return a.level > b.level; }); std::ofstream fradius;
  //   fradius.open(filepath + ".rad");
  //   for (auto it : Rstat)
  //     fradius << it.found << " " << it.nRu << " " << it.nEu << " " <<
  //     it.nCu
  //     << " " << it.nRv << " " << it.nEv << " "
  //             << it.nCv << " " << it.level << std::endl;
  //   fradius.close();
  // }
}
inline void
SCCWN::placeEdges(parlay::sequence<std::pair<size_t, size_t>> &edges,
                  size_t l) {
  for (auto it : edges) {
    leaves[it.first]->insertToLeaf(it.second, l);
    leaves[it.second]->insertToLeaf(it.first, l);
  }
}
inline std::tuple<bool, size_t, size_t>
SCCWN::fetchEdge(std::queue<localTree *> &Q, size_t l) {
  if (Q.empty())
    return std::make_tuple(false, 0, 0);
  auto node = Q.front();
  auto e = localTree::fetchEdge(node, l);
  while (!std::get<0>(e)) {
    Q.pop();
    if (Q.empty())
      return std::make_tuple(false, 0, 0);
    node = Q.front();
    e = localTree::fetchEdge(node, l);
  }
  return e;
}
