#include "alloc.h"
#include "dycon/helpers/union_find.hpp"
#include "fetchQueue.hpp"
#include "graph.hpp"
#include "leaf.hpp"
#include "localTree.hpp"
#include "parlay/internal/group_by.h"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <algorithm>
#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iterator>
#include <optional>
#include <utility>
#include <vector>
class SCCWN {
private:
  std::vector<std::pair<uint32_t, uint32_t>> Eu, Ev; // fetched edge
  absl::flat_hash_map<localTree *, std::pair<vertex, vertex>> Ru,
      Rv; // visited node
  fetchQueue<localTree *> LTNodeQ_U,
      LTNodeQ_V; // localTree nodes ready to fetch
  fetchQueue<vertex, edge_set::iterator> lfQ_U,
      lfQ_V; // vertices ready to fetch
  absl::flat_hash_set<vertex> Lu, Lv;
  // above are the data structures used to do the tick tock replacement edge
  // search

  static std::tuple<bool, uint32_t, uint32_t>
  fetchEdge(fetchQueue<localTree *> &Q, uint32_t l);
  std::optional<std::pair<vertex, vertex>>
  fetchEdge(fetchQueue<localTree *> &LTNodeQ,
            fetchQueue<vertex, edge_set::iterator> &lfQ, uint32_t l);
  void restoreBitMap(fetchQueue<vertex, edge_set::iterator> &lfQ, uint32_t l,
                     bool nval);
  static localTree *pushDown(std::vector<localTree *> &pu, uint32_t uter,
                             std::vector<localTree *> &pv, uint32_t vter);
  void placeEdges(std::vector<std::pair<uint32_t, uint32_t>> &edges,
                  uint32_t l);
  void changeLevel(std::vector<std::pair<uint32_t, uint32_t>> &edges,
                   uint32_t oval, uint32_t nval);
  void printNodes(std::vector<localTree *> &Nodes) {
    std::copy(Nodes.begin(), Nodes.end(),
              std::ostream_iterator<localTree *>(std::cout, ","));
    std::cout << "\n\n";
  }
  void GC(bool clear, std::atomic<size_t> &mem_usage);

public:
  uint32_t n;
  static uint32_t lmax;
  std::vector<localTree *> leaves;
  uint32_t self_edge[2] = {0, 0};
  absl::flat_hash_set<std::pair<uint32_t, uint32_t>> nonTreeEdge;
  absl::flat_hash_set<std::pair<uint32_t, uint32_t>> TreeEdge;
  uint32_t NTE = 0;
  uint32_t SLE = 0;
  uint32_t TE = 0;
  SCCWN(uint32_t _n) : n(_n), leaves(std::vector<localTree *>(n, nullptr)) {
    rankTree::r_alloc = new type_allocator<rankTree>(n);
    localTree::l_alloc = new type_allocator<localTree>(n);
    leaf::vector_alloc = new type_allocator<edge_set>(n);
    leaves = std::vector<localTree *>(n);
    for (uint32_t i = 0; i < n; i++)
      leaves[i] = localTree::l_alloc->create(i);
  }
  ~SCCWN() {
    std::atomic<std::size_t> mem_usage = 0;
    parlay::execute_with_scheduler(1, [&]() { GC(true, mem_usage); });
    rankTree::r_alloc->~type_allocator();
    localTree::l_alloc->~type_allocator();
    leaf::vector_alloc->~type_allocator();
  };
  void insert(uint32_t u, uint32_t v) { insertToLCA(u, v); }
  void insertToLCA(uint32_t u, uint32_t v);
  void insertToRoot(uint32_t u, uint32_t v);
  void insertToBlock(uint32_t u, uint32_t v);
  bool is_connected(uint32_t u, uint32_t v);
  void remove(uint32_t u, uint32_t v);
  size_t getMemUsage() {
    std::atomic<std::size_t> mem_usage = 0;
    GC(false, mem_usage);
    mem_usage += rankTree::r_alloc->used_mem();
    mem_usage += localTree::l_alloc->used_mem();
    mem_usage += leaf::vector_alloc->used_mem();
    parlay::parallel_for(0, leaves.size(), [&](size_t i) {
      mem_usage += localTree::getLeafSpace(leaves[i]);
    });
    mem_usage += sizeof(SCCWN);
    mem_usage += TreeEdge.bucket_count() * sizeof(std::pair<vertex, vertex>);
    mem_usage += nonTreeEdge.bucket_count() * sizeof(std::pair<vertex, vertex>);
    return (size_t)mem_usage;
  }
  void getInfo(vertex u, vertex v) {
    std::cout << u << " " << leaves[u]->getNGHS() << " " << v << " "
              << leaves[v]->getNGHS() << std::endl;
    auto Cu = localTree::getRoot(leaves[u]);
    auto Cv = localTree::getRoot(leaves[v]);
    if (Cu != Cv)
      return;
    std::cout << u << " " << Cu << " " << Cu->getLevel() << " " << Cu->getMap()
              << " " << Cu->getSize() << " " << v << " " << Cv << " "
              << Cv->getLevel() << " " << Cv->getMap() << " " << Cv->getSize()
              << std::endl;
    Cu = localTree::getParent(leaves[u]);
    Cv = localTree::getParent(leaves[v]);
    std::cout << u << " " << Cu << " " << Cu->getLevel() << " " << Cu->getMap()
              << " " << Cu->getSize() << " " << v << " " << Cv << " "
              << Cv->getLevel() << " " << Cv->getMap() << " " << Cv->getSize()
              << std::endl;
  }
};
inline uint32_t SCCWN::lmax = 63;
inline void SCCWN::insertToRoot(uint32_t u, uint32_t v) {
  if (u > v)
    std::swap(u, v);
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
  if (Cu != Cv) {
    localTree::merge(Cu, Cv);
    TreeEdge.emplace(std::pair(u, v));
  } else
    nonTreeEdge.emplace(std::pair(u, v));
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
        // auto P = localTree::getParent(Cu);
        // uint32_t l = (P == nullptr) ? Cu->getLevel() : P->getLevel();
        uint32_t l = Cu->getLevel();
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
  if (!TreeEdge.contains(std::pair(u, v))) {
    nonTreeEdge.erase(std::pair(u, v));
    // NTE++;
    return;
  }
  TreeEdge.erase(std::pair(u, v));
  auto Cu = localTree::getLevelNode(leaves[u], l);
  auto Cv = localTree::getLevelNode(leaves[v], l);
  assert(Cu != nullptr && Cv != nullptr);
  if (Cu == Cv) {
    // SLE++;
    return;
  }
  // TE++;
  auto CP = localTree::getParent(Cu);
  assert(localTree::getParent(Cu) == localTree::getParent(Cv));
  auto init = [](fetchQueue<localTree *> &LTNodeQ,
                 fetchQueue<vertex, edge_set::iterator> &lfQ,
                 std::vector<std::pair<uint32_t, uint32_t>> &E,
                 absl::flat_hash_set<vertex> &L,
                 absl::flat_hash_map<localTree *, std::pair<vertex, vertex>> &R,
                 localTree *C) -> void {
    LTNodeQ.clear();
    lfQ.clear();
    E.clear();
    R.clear();
    L.clear();
    LTNodeQ.push(C);
    R.emplace(C, std::pair(0, 0));
    E.reserve(128);
  };
  while (l != 0) {
    init(LTNodeQ_U, lfQ_U, Eu, Lu, Ru, Cu);
    init(LTNodeQ_V, lfQ_V, Ev, Lv, Rv, Cv);
    auto nCu = Cu->getSize();
    auto nCv = Cv->getSize();
    assert(Cu != Cv);
    while (true) {
      if (nCu <= CP->getSize() / 2) {
        auto eu = fetchEdge(LTNodeQ_U, lfQ_U, l);
        if (eu != std::nullopt) {
          if (leaves[eu->second]->getMap()[l] == 1) {
            auto Cuv = localTree::getLevelNode(leaves[eu->second], l);
            assert(Cuv != nullptr);
            assert(CP == nullptr || localTree::getParent(Cuv) == CP);
            if (Rv.find(Cuv) != Rv.end()) {
              auto ev = Rv.find(Cuv)->second;
              if (Eu.empty()) {
                // don't push nCu+nCv may violate size
                // return;
              } else if (nCu <= nCv) {
                auto C = localTree::splitFromParent(CP, LTNodeQ_U);
                localTree::addChild(CP, C);
                restoreBitMap(lfQ_V, l, 1);
                changeLevel(Eu, l, C->getLevel());
              } else {
                auto C = localTree::splitFromParent(CP, LTNodeQ_V);
                localTree::addChild(CP, C);
                restoreBitMap(lfQ_U, l, 1);
                changeLevel(Ev, l, C->getLevel());
              }
              return;
            } else {
              if (Ru.find(Cuv) == Ru.end()) {
                if (eu->first > eu->second)
                  std::swap(eu->first, eu->second);
                Ru.emplace(Cuv, *eu);
                LTNodeQ_U.push(Cuv);
                nCu += Cuv->getSize();
              }
              if (!Lu.contains(eu->second) &&
                  leaves[eu->second]->getMap()[l] == 1) {
                Lu.emplace(eu->second);
                lfQ_U.push(eu->second);
              }
              Eu.emplace_back(std::pair(eu->first, eu->second));
            }
          }
        } else {
          auto GP = localTree::getParent(CP);
          localTree::deleteFromParent(CP);
          localTree *_CP;
          if (Eu.empty()) {
            _CP = *LTNodeQ_U.begin();
            localTree::deleteFromParent(*LTNodeQ_U.begin());
            if (localTree::ifSingleton(CP) == true &&
                CP->getMap()[l] == false) {
              localTree::deleteFromParent(*LTNodeQ_V.begin());
              localTree::l_alloc->free(CP); // delete CP;
              CP = *LTNodeQ_V.begin();
            }
          } else if (nCu <= nCv) {
            _CP = localTree::splitFromParent(CP, LTNodeQ_U);
            restoreBitMap(lfQ_V, l, 1);
            changeLevel(Eu, l, _CP->getLevel());
          } else {
            _CP = localTree::l_alloc->create();
            _CP->setLevel(l);
            localTree::deleteFromParent(CP, LTNodeQ_U);
            localTree::addChildren(_CP, LTNodeQ_U);
            auto oldMap = CP->getMap();
            auto C = localTree::splitFromParent(CP, LTNodeQ_V);
            if (oldMap[l] == false) {
              localTree::l_alloc->free(CP); // delete CP;
              CP = C;
            } else
              localTree::addChild(CP, C);
            restoreBitMap(lfQ_U, l, 1);
            changeLevel(Ev, l, C->getLevel());
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
        auto ev = fetchEdge(LTNodeQ_V, lfQ_V, l);
        if (ev != std::nullopt) {
          if (leaves[ev->second]->getMap()[l] == 1) {
            auto Cuv = localTree::getLevelNode(leaves[ev->second], l);
            assert(Cuv != nullptr);
            assert(CP == nullptr || localTree::getParent(Cuv) == CP);
            if (Ru.find(Cuv) != Ru.end()) {
              if (Ev.empty()) {
                restoreBitMap(lfQ_U, l, 1);
              } else if (nCu <= nCv) {
                auto C = localTree::splitFromParent(CP, LTNodeQ_U);
                localTree::addChild(CP, C);
                restoreBitMap(lfQ_V, l, 1);
                changeLevel(Eu, l, C->getLevel());
              } else {
                auto C = localTree::splitFromParent(CP, LTNodeQ_V);
                localTree::addChild(CP, C);
                restoreBitMap(lfQ_U, l, 1);
                changeLevel(Ev, l, C->getLevel());
              }
              if (ev->first > ev->second)
                std::swap(ev->first, ev->second);
              return;
            } else {
              if (Rv.find(Cuv) == Rv.end()) {
                if (ev->first > ev->second)
                  std::swap(ev->first, ev->second);
                Rv.emplace(Cuv, *ev);
                LTNodeQ_V.push(Cuv);
                nCv += Cuv->getSize();
              }
              if (!Lv.contains(ev->second) &&
                  leaves[ev->second]->getMap()[l] == 1) {
                Lv.emplace(ev->second);
                lfQ_V.push(ev->second);
              }
              Ev.emplace_back(std::pair(ev->first, ev->second));
            }
          }
        } else {
          auto GP = localTree::getParent(CP);
          localTree::deleteFromParent(CP);
          localTree *_CP; // = localTree::l_alloc->create();
          if (Ev.empty()) {
            _CP = *LTNodeQ_V.begin();
            localTree::deleteFromParent(*LTNodeQ_V.begin());
            restoreBitMap(lfQ_U, l, 1);
          } else if (nCv <= nCu) {
            _CP = localTree::splitFromParent(CP, LTNodeQ_V);
            restoreBitMap(lfQ_U, l, 1);
            changeLevel(Ev, l, _CP->getLevel());
          } else {
            _CP = localTree::l_alloc->create();
            _CP->setLevel(l);
            localTree::deleteFromParent(CP, LTNodeQ_V);
            localTree::addChildren(_CP, LTNodeQ_V);
            auto oldMap = CP->getMap();
            auto C = localTree::splitFromParent(CP, LTNodeQ_U);
            if (oldMap[l] == false) {
              localTree::l_alloc->free(CP); // delete CP;
              CP = C;
            } else
              localTree::addChild(CP, C);
            restoreBitMap(lfQ_V, l, 1);
            changeLevel(Eu, l, C->getLevel());
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
inline void SCCWN::placeEdges(std::vector<std::pair<uint32_t, uint32_t>> &edges,
                              uint32_t l) {
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
SCCWN::fetchEdge(fetchQueue<localTree *> &LTNodeQ,
                 fetchQueue<vertex, edge_set::iterator> &lfQ, uint32_t l) {
  if (!lfQ.empty()) {
    if (leaves[lfQ.front()]->getMap()[l] == 1) {
      if (lfQ.pos != lfQ.tail) {
        vertex v = *(lfQ.pos);
        lfQ.pos++;
        vertex test1 = std::min(lfQ.front(), v);
        vertex test2 = std::max(lfQ.front(), v);
        TreeEdge.emplace(std::pair(test1, test2));
        return std::pair(lfQ.front(), v);
      } else {
        leaves[lfQ.front()]->setBitMap(l, 0);
        localTree::updateBitMap(leaves[lfQ.front()]);
      }
    }
    lfQ.pop();
  }
  while (!lfQ.empty() && leaves[lfQ.front()]->getMap()[l] == 0)
    lfQ.pop();
  if (!lfQ.empty()) {
    lfQ.pos = localTree::getEdgeSet(leaves[lfQ.front()], l)->begin();
    lfQ.tail = localTree::getEdgeSet(leaves[lfQ.front()], l)->end();
    vertex v = *(lfQ.pos);
    lfQ.pos++;
    vertex test1 = std::min(lfQ.front(), v);
    vertex test2 = std::max(lfQ.front(), v);
    TreeEdge.emplace(std::pair(test1, test2));
    return std::pair(lfQ.front(), v);
  }
  while (!LTNodeQ.empty()) {
    auto LTNode = LTNodeQ.front();
    if (LTNode->getMap()[l] == 0) {
      LTNodeQ.pop();
    } else {
      auto fetched = localTree::fetchLeaf(LTNode, l);
      assert(fetched != std::nullopt);
      lfQ.pos = fetched->second->begin();
      lfQ.tail = fetched->second->end();
      vertex u = fetched->first;
      vertex v = *(lfQ.pos);
      lfQ.pos++;
      lfQ.push(u);
      vertex test1 = std::min(u, v);
      vertex test2 = std::max(u, v);
      TreeEdge.emplace(std::pair(test1, test2));
      return std::pair(u, v);
    }
  }
  return std::nullopt;
}
inline void SCCWN::restoreBitMap(fetchQueue<vertex, edge_set::iterator> &lfQ,
                                 uint32_t l, bool nval) {
  for (auto it : lfQ) {
    if (leaves[it]->getMap()[l] != nval) {
      leaves[it]->setBitMap(l, nval);
      localTree::updateBitMap(leaves[it]);
    }
  }
}
inline void
SCCWN::changeLevel(std::vector<std::pair<uint32_t, uint32_t>> &edges,
                   uint32_t oval, uint32_t nval) {
  for (auto it : edges) {
    vertex test1 = std::min(it.first, it.second);
    vertex test2 = std::max(it.first, it.second);
    leaves[it.first]->deleteEdge(it.second, oval);
    leaves[it.first]->insertToLeaf(it.second, nval);
    leaves[it.second]->deleteEdge(it.first, oval);
    leaves[it.second]->insertToLeaf(it.first, nval);
  }
}
inline void SCCWN::GC(bool clear, std::atomic<size_t> &mem_usage) {
  parlay::sequence<localTree *> roots(leaves.size());
  parlay::parallel_for(0, leaves.size(), [&](auto i) {
    if (leaves[i])
      roots[i] = localTree::getRoot(leaves[i]);
    else
      roots[i] = nullptr;
  });
  auto r = parlay::remove_duplicates(roots);
  parlay::parallel_for(0, r.size(), [&](size_t i) {
    if (r[i]) {
      localTree::topDown(r[i], clear, mem_usage);
    }
  });
}