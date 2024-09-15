#include "alloc.h"
#include "dycon/helpers/union_find.hpp"
#include "fetchQueue.hpp"
#include "graph.hpp"
#include "leaf.hpp"
#include "localTree.hpp"
#include "parlay/internal/group_by.h"
#include "parlay/sequence.h"
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>
class SCCWN {
private:
  struct fetchLeaf {
    vertex id;
    absl::flat_hash_set<vertex> *edges;
    absl::flat_hash_set<vertex>::iterator eit;
  };
  static std::tuple<bool, uint32_t, uint32_t>
  fetchEdge(fetchQueue<localTree *> &Q, uint32_t l);
  std::optional<std::pair<vertex, vertex>>
  fetchEdge(fetchQueue<localTree *> &LTNodeQ, fetchQueue<fetchLeaf> &lfQ,
            uint32_t l);
  static localTree *pushDown(std::vector<localTree *> &pu, uint32_t uter,
                             std::vector<localTree *> &pv, uint32_t vter);
  void placeEdges(std::vector<std::pair<uint32_t, uint32_t>> &edges, uint32_t l,
                  bool undirected = false);
  void printNodes(std::vector<localTree *> &Nodes) {
    std::copy(Nodes.begin(), Nodes.end(),
              std::ostream_iterator<localTree *>(std::cout, ","));
    std::cout << "\n\n";
  }

public:
  uint32_t n;
  static uint32_t lmax;
  std::vector<localTree *> leaves;
  uint32_t self_edge[2] = {0, 0};
  absl::flat_hash_set<std::pair<uint32_t, uint32_t>> nonTreeEdge;
  absl::flat_hash_set<std::pair<uint32_t, uint32_t>> TreeEdge;
  SCCWN(uint32_t _n) : n(_n), leaves(std::vector<localTree *>(n, nullptr)) {
    parlay::internal::timer t;
    rankTree::r_alloc = new type_allocator<rankTree>(n);
    localTree::l_alloc = new type_allocator<localTree>(n);
    leaf::vector_alloc = new type_allocator<absl::flat_hash_set<vertex>>(n);
    nonTreeEdge.reserve(5 * n);
    TreeEdge.reserve(n);
    leaves = std::vector<localTree *>(n);
    for (uint32_t i = 0; i < n; i++)
      leaves[i] = localTree::l_alloc->create(i);

    t.next("constructor");
  }
  ~SCCWN() {
    run_stat("./", false, true, false);
    rankTree::r_alloc->~type_allocator();
    localTree::l_alloc->~type_allocator();
  };
  void insert(uint32_t u, uint32_t v) { insertToLCA(u, v); }
  void insertToLCA(uint32_t u, uint32_t v);
  void insertToRoot(uint32_t u, uint32_t v);
  void insertToBlock(uint32_t u, uint32_t v);
  bool is_connected(uint32_t u, uint32_t v);
  void remove(uint32_t u, uint32_t v);
  void run_stat(std::string filepath, bool verbose, bool clear, bool stat);
  void checkLevel() {
    for (uint32_t i = 0; i < n; i++) {
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
inline uint32_t SCCWN::lmax = 63;
inline void SCCWN::insertToRoot(uint32_t u, uint32_t v) {
  auto g = [&](uint32_t &u) -> localTree * {
    localTree *r = localTree::getRoot(leaves[u]);
    if (r->getLevel() == lmax)
      return r;
    auto p = localTree::l_alloc->create();
    p->setLevel(lmax);
    localTree::addChild(p, r);
    return p;
  };
  if (leaves[u] == nullptr)
    leaves[u] = localTree::l_alloc->create(u);
  if (leaves[v] == nullptr)
    leaves[v] = localTree::l_alloc->create(v);
  auto Cu = g(u);
  auto Cv = g(v);
  if (Cu != Cv)
    localTree::merge(Cu, Cv);
  leaves[u]->insertToLeaf(v, lmax);
  leaves[v]->insertToLeaf(u, lmax);
}
inline void SCCWN::insertToLCA(uint32_t u, uint32_t v) {
  if (leaves[u] == nullptr)
    leaves[u] = localTree::l_alloc->create(u);
  if (leaves[v] == nullptr)
    leaves[v] = localTree::l_alloc->create(v);
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
        uint32_t l = (P == nullptr) ? Cu->getLevel() : P->getLevel();
        leaves[u]->insertToLeaf(v, l);
        leaves[v]->insertToLeaf(u, l);
        if (u > v)
          std::swap(u, v);
        nonTreeEdge.emplace(std::pair(u, v));
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
  uint32_t l;
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
    auto r = localTree::l_alloc->create(Cu, Cv);
    l = r->getLevel();
  }
  leaves[u]->insertToLeaf(v, l);
  leaves[v]->insertToLeaf(u, l);
  if (u > v)
    std::swap(u, v);
  TreeEdge.emplace(std::pair(u, v));
}
inline void SCCWN::insertToBlock(uint32_t u, uint32_t v) {
  if (leaves[u] == nullptr)
    leaves[u] = localTree::l_alloc->create(u);
  if (leaves[v] == nullptr)
    leaves[v] = localTree::l_alloc->create(v);
  auto pv = localTree::getRootPath(leaves[v]);
  auto pu = localTree::getRootPath(leaves[u]);
  uint32_t uLen = pu.size();
  uint32_t vLen = pv.size();
  localTree *rootu = pu[uLen - 1];
  localTree *rootv = pv[vLen - 1];
  localTree *np = nullptr;
  if (rootu != rootv) {
    if (rootu->getSize() + rootv->getSize() >
        (1 << std::max(rootu->getLevel(), rootv->getLevel())))
      np = rootu->getLevel() >= rootv->getLevel()
               ? localTree::l_alloc->create(rootu, rootv)
               : localTree::l_alloc->create(rootv, rootu);
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
                 ? localTree::l_alloc->create(pu[uLen - 1], pv[vLen - 1])
                 : localTree::l_alloc->create(pv[vLen - 1], pu[uLen - 1]);
        localTree::addChild(lca, np);
      } else {
        auto temp = localTree::getParent(lca);
        np = temp == nullptr ? lca : temp;
      }
    } else
      np = pushDown(pu, uLen - 1, pv, vLen - 1);
  }
  leaves[u]->insertToLeaf(v, np->getLevel());
  leaves[v]->insertToLeaf(u, np->getLevel());
}
inline localTree *SCCWN::pushDown(std::vector<localTree *> &pu, uint32_t uter,
                                  std::vector<localTree *> &pv, uint32_t vter) {
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
    }
  }
  assert(localTree::getParent(pu[uter]) == localTree::getParent(pv[vter]));
  return localTree::getParent(pu[uter]);
}
inline void SCCWN::remove(uint32_t u, uint32_t v) {
  // std::cout << u << " " << v << std::endl;
  assert(leaves[u]->getEdgeLevel(v) == leaves[v]->getEdgeLevel(u));
  uint32_t l = leaves[u]->getSize() < leaves[v]->getSize()
                   ? leaves[u]->getEdgeLevel(v)
                   : leaves[v]->getEdgeLevel(u);
  leaves[u]->deleteEdge(v, l);
  leaves[v]->deleteEdge(u, l);
  auto Cu = localTree::getLevelNode(leaves[u], l);
  auto Cv = localTree::getLevelNode(leaves[v], l);
  assert(Cu != nullptr && Cv != nullptr);
  if (Cu == Cv) {
    self_edge[1]++;
    return;
  }
  self_edge[0]++;
  auto CP = localTree::getParent(Cu);
  assert(localTree::getParent(Cu) == localTree::getParent(Cv));

  // std::queue<localTree *> Qu, Qv;                    // ready to fetch
  fetchQueue<localTree *> Qu, Qv;
  std::vector<std::pair<uint32_t, uint32_t>> Eu, Ev; // fetched edge
  absl::flat_hash_set<localTree *> Ru, Rv;           // visited node
  fetchQueue<localTree *> LTNodeQ_U,
      LTNodeQ_V;                      // localTree nodes ready to fetch
  fetchQueue<fetchLeaf> lfQ_U, lfQ_V; // vertices ready to fetch
  auto init = [](fetchQueue<localTree *> &Q, fetchQueue<localTree *> &LTNodeQ,
                 fetchQueue<fetchLeaf> &lfQ,
                 std::vector<std::pair<uint32_t, uint32_t>> &E,
                 absl::flat_hash_set<localTree *> &R, localTree *C) -> void {
    Q = fetchQueue<localTree *>();
    LTNodeQ = fetchQueue<localTree *>();
    lfQ = fetchQueue<fetchLeaf>();
    E = std::vector<std::pair<uint32_t, uint32_t>>();
    R = absl::flat_hash_set<localTree *>();
    LTNodeQ.push(C);
    Q.push(C);
    R.insert(C);
    E.reserve(128);
  };
  while (l != 0) {
    init(Qu, LTNodeQ_U, lfQ_U, Eu, Ru, Cu);
    init(Qv, LTNodeQ_V, lfQ_V, Ev, Rv, Cv);
    auto nCu = Cu->getSize();
    auto nCv = Cv->getSize();
    assert(Cu != Cv);
    while (true) {
      if (nCu <= CP->getSize() / 2) {
        auto eu = fetchEdge(Qu, l);
        if (std::get<0>(eu) == true) {
          auto Cuv = localTree::getLevelNode(leaves[std::get<2>(eu)], l);
          assert(Cuv != nullptr);
          assert(CP == nullptr || localTree::getParent(Cuv) == CP);
          if (Rv.find(Cuv) != Rv.end()) {
            if (Eu.empty()) {
              // don't push nCu+nCv may violate size
              return;
            } else if (nCu <= nCv) {
              auto C = localTree::splitFromParent(CP, Qu);
              localTree::addChild(CP, C);
              placeEdges(Eu, C->getLevel(), true);
              placeEdges(Ev, l);
            } else {
              auto C = localTree::splitFromParent(CP, Qv);
              localTree::addChild(CP, C);
              placeEdges(Eu, l);
              placeEdges(Ev, C->getLevel(), true);
            }
            return;
          } else {
            if (Ru.find(Cuv) == Ru.end()) {
              Ru.insert(Cuv);
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
            localTree::deleteFromParent(*Ru.begin());
            _CP = *Ru.begin();
            if (localTree::ifSingleton(CP) == true &&
                CP->getMap()[l] == false) {
              localTree::deleteFromParent(*Rv.begin());
              localTree::l_alloc->free(CP); // delete CP;
              CP = *Rv.begin();
            }
          } else if (nCu <= nCv) {
            _CP = localTree::splitFromParent(CP, Qu);
            placeEdges(Eu, _CP->getLevel(), true);
            placeEdges(Ev, l);
          } else {
            _CP = localTree::l_alloc->create();
            _CP->setLevel(l);
            localTree::deleteFromParent(CP, Qu);
            localTree::addChildren(_CP, Qu);
            auto oldMap = CP->getMap();
            auto C = localTree::splitFromParent(CP, Qv);
            if (oldMap[l] == false) {
              localTree::l_alloc->free(CP); // delete CP;
              CP = C;
            } else
              localTree::addChild(CP, C);
            placeEdges(Eu, l);
            placeEdges(Ev, C->getLevel(), true);
          }
          localTree::add2Children(GP, CP, _CP);
          Cu = _CP;
          Cv = CP;
          CP = GP;
          l = CP ? CP->getLevel() : 0;
          break;
        }
      }
      if (nCv <= CP->getSize() / 2) {
        auto ev = fetchEdge(Qv, l);
        if (std::get<0>(ev) == true) {
          auto Cuv = localTree::getLevelNode(leaves[std::get<2>(ev)], l);
          assert(Cuv != nullptr);
          assert(CP == nullptr || localTree::getParent(Cuv) == CP);
          if (Ru.find(Cuv) != Ru.end()) {
            if (Ev.empty()) {
              placeEdges(Eu, l);
            } else if (nCu <= nCv) {
              auto C = localTree::splitFromParent(CP, Qu);
              localTree::addChild(CP, C);
              placeEdges(Eu, C->getLevel(), true);
              placeEdges(Ev, l);
            } else {
              auto C = localTree::splitFromParent(CP, Qv);
              localTree::addChild(CP, C);
              placeEdges(Eu, l);
              placeEdges(Ev, C->getLevel(), true);
            }
            return;
          } else {
            if (Rv.find(Cuv) == Rv.end()) {
              Rv.insert(Cuv);
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
          localTree *_CP; // = localTree::l_alloc->create();
          if (Ev.empty()) {
            localTree::deleteFromParent(*Rv.begin());
            _CP = *Rv.begin();
            placeEdges(Eu, l);
          } else if (nCv <= nCu) {
            _CP = localTree::splitFromParent(CP, Qv);
            placeEdges(Eu, l);
            placeEdges(Ev, _CP->getLevel(), true);
          } else {
            _CP = localTree::l_alloc->create();
            _CP->setLevel(l);
            localTree::deleteFromParent(CP, Qv);
            localTree::addChildren(_CP, Qv);
            auto oldMap = CP->getMap();
            auto C = localTree::splitFromParent(CP, Qu);
            if (oldMap[l] == false) {
              localTree::l_alloc->free(CP); // delete CP;
              CP = C;
            } else
              localTree::addChild(CP, C);
            placeEdges(Eu, C->getLevel(), true);
            placeEdges(Ev, l);
          }
          localTree::add2Children(GP, CP, _CP);
          Cu = CP;
          Cv = _CP;
          CP = GP;
          l = CP ? CP->getLevel() : 0;

          break;
        }
      }
    }
  }
}
inline bool SCCWN::is_connected(uint32_t u, uint32_t v) {
  return localTree::getRoot(leaves[u]) == localTree::getRoot(leaves[v]);
}

inline void SCCWN::run_stat(std::string filepath, bool verbose = false,
                            bool clear = false, bool stat = true) {
  std::vector<localTree *> roots(leaves.size(), nullptr);
  std::vector<localTree *> parents(leaves.size(), nullptr);
  parlay::parallel_for(0, roots.size(), [&](uint32_t i) {
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
  parlay::sequence<localTree *> r(roots.begin(), roots.end());
  r = parlay::remove_duplicates(r);
  parlay::parallel_for(0, r.size(), [&](uint32_t i) {
    if (r[i]) {
      parlay::sequence<stats> info;
      localTree::traverseTopDown(r[i], clear, verbose, stat, info);
    }
  });
}
inline void SCCWN::placeEdges(std::vector<std::pair<uint32_t, uint32_t>> &edges,
                              uint32_t l, bool undirected) {
  for (auto it : edges) {
    leaves[it.first]->insertToLeaf(it.second, l);
    leaves[it.second]->insertToLeaf(it.first, l);
  }
}
inline std::tuple<bool, uint32_t, uint32_t>
SCCWN::fetchEdge(fetchQueue<localTree *> &Q, uint32_t l) {
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
inline std::optional<std::pair<vertex, vertex>>
SCCWN::fetchEdge(fetchQueue<localTree *> &LTNodeQ, fetchQueue<fetchLeaf> &lfQ,
                 uint32_t l) {
  while (!lfQ.empty()) {
    auto e = lfQ.front();
    if (/*leaves[e.id]->getMap()[l] == 1 && */ e.eit != e.edges->end()) {
      vertex v = *(e.eit);
      e.eit++;
      return std::pair(e.id, v);
    }
    leaves[e.id]->setBitMap(l, 0);
    localTree::updateBitMap(leaves[e.id]);
    lfQ.pop();
  }
  while (!LTNodeQ.empty()) {
    auto LTNode = LTNodeQ.front();
    if (LTNode->getMap()[l] == 0) {
      LTNodeQ.pop();
    } else {
      auto fetched = localTree::fetchLeaf(LTNode, l);
      assert(fetched != std::nullopt);
      auto eit = fetched->second->begin();
      vertex u = fetched->first;
      vertex v = *eit;
      eit++;
      const fetchLeaf fl = {.id = u, .edges = fetched->second, .eit = eit};
      lfQ.push(std::move(fl));
      return std::pair(u, v);
    }
  }
  return std::nullopt;
}
