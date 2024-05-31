#include "localTree.hpp"
#include <fstream>
#include <queue>
#include <unordered_set>
class SCCWN {
private:
  static std::tuple<bool, size_t, size_t> fetchEdge(std::queue<localTree *> &Q,
                                                    size_t l);
  void placeEdges(parlay::sequence<std::pair<size_t, size_t>> &edges, size_t l);
  void printNodes(parlay::sequence<localTree *> &Nodes) {
    std::copy(Nodes.begin(), Nodes.end(),
              std::ostream_iterator<localTree *>(std::cout, ","));
    std::cout << "\n\n";
  }

public:
  size_t n;
  static size_t lmax;
  static bool blocked_insert;
  parlay::sequence<localTree *> leaves;
  SCCWN(size_t _n) : n(_n), leaves(parlay::sequence<localTree *>(n, nullptr)) {}
  // ~SCCWN() { run_stat("./", true, false, false); };
  void insert(size_t u, size_t v);
  bool is_connected(size_t u, size_t v);
  void remove(size_t u, size_t v);
  void run_stat(std::string filepath, bool verbose, bool clear, bool stat);
  void print_cg_sizes();
  int64_t space();
  void checkLevel() {
    for (size_t i = 0; i < n; i++) {
      if (leaves[i] != nullptr) {
        auto r = leaves[i];
        while (localTree::getParent(r) != nullptr) {
          if (localTree::getParent(r)->getLevel() <= r->level) {
            std::cout << "parent level = "
                      << localTree::getParent(r)->getLevel()
                      << " parent size = " << localTree::getParent(r)->getSize()
                      << " child level = " << r->level
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
inline bool SCCWN::blocked_insert = false;

void SCCWN::print_cg_sizes() {
  size_t total = 0;
  size_t num_cg = 0;
  size_t total_es = 0;
  size_t num_cg_es = 0;
  size_t max = 0;
  std::set<localTree *> visited_cg;
  for (size_t i = 0; i < n; i++) {
    if (leaves[i] == nullptr)
      continue;
    auto lTree = localTree::getParent(leaves[i]);
    while (lTree) {
      auto cg = lTree;
      if (visited_cg.find(cg) == visited_cg.end()) {
        size_t size = cg->get_cluster_graph_size();
        total += size;
        num_cg += 1;
        if (size > 1) {
          total_es += size;
          num_cg_es += 1;
        }
        max = std::max(max, size);
        visited_cg.insert(cg);
      }
      lTree = localTree::getParent(lTree);
    }
  }
  std::cout << "Cluster Graph Sizes:" << std::endl;
  std::cout << "TOTAL SIZE: " << total;
  std::cout << " NUM CGS: " << num_cg << std::endl;
  std::cout << "AVG: " << (float)total / (float)num_cg;
  std::cout << " AVG_EXCL_SING: " << (float)total_es / (float)num_cg_es;
  std::cout << " MAX: " << max << std::endl;
}

int64_t SCCWN::space() {
  int64_t space = 0;
  space += sizeof(std::vector<localTree *>);
  space += leaves.size() * sizeof(localTree *);
  space += 2 * sizeof(size_t);
  std::set<rankTree *> visited_rnodes;
  std::set<localTree *> visited_lnodes;
  for (localTree *leaf : leaves) {
    if (leaf == nullptr)
      continue;
    bool continueloop = false;
    space += leaf->space();
    space += leaf->vertex->space();
    rankTree *rTree = leaf->parent;
    if (!rTree)
      continue;
    visited_rnodes.insert(rTree);
    while (rTree->parent) {
      rTree = rTree->parent;
      if (visited_rnodes.find(rTree) != visited_rnodes.end()) {
        continueloop = true;
        break;
      }
      visited_rnodes.insert(rTree);
    }
    if (continueloop)
      continue;
    localTree *lTree = rTree->Node;
    if (visited_lnodes.find(lTree) != visited_lnodes.end())
      continue;
    visited_lnodes.insert(lTree);
    while (lTree->parent) {
      rTree = lTree->parent;
      visited_rnodes.insert(rTree);
      while (rTree->parent) {
        rTree = rTree->parent;
        if (visited_rnodes.find(rTree) != visited_rnodes.end()) {
          continueloop = true;
          break;
        }
        visited_rnodes.insert(rTree);
      }
      if (continueloop)
        break;
      lTree = rTree->Node;
      if (visited_lnodes.find(lTree) != visited_lnodes.end())
        break;
      visited_lnodes.insert(lTree);
    }
  }
  space += visited_rnodes.size() * sizeof(rankTree);
  for (localTree *lTree : visited_lnodes)
    space += lTree->space();
  return space;
}

inline void SCCWN::insert(size_t u, size_t v) {
  if (leaves[u] == nullptr)
    leaves[u] = new localTree(u);
  if (leaves[v] == nullptr)
    leaves[v] = new localTree(v);
  auto pv = localTree::getRootPath(leaves[v]);
  auto pu = localTree::getRootPath(leaves[u]);
  auto iu = pu.rbegin();
  auto iv = pv.rbegin();
  size_t l;
  auto Cu = *iu;
  auto Cv = *iv;
  if (*iu != *iv) {
    size_t max_l = std::max(Cu->getLevel(), Cv->getLevel());
    if (Cu->getSize() + Cv->getSize() <= (1 << max_l)) {
      if (Cu->getLevel() > Cv->getLevel()) {
        localTree::addChild(Cu, Cv);
        l = Cu->getLevel();
        iu++;
        Cu = *iu;
      } else if (Cu->getLevel() < Cv->getLevel()) {
        localTree::addChild(Cv, Cu);
        l = Cv->getLevel();
        iv++;
        Cv = *iv;
      } else {
        localTree::merge(Cu, Cv);
        l = Cu->getLevel();
        iu++;
        iv++;
        Cu = *iu;
        Cv = *iv;
      }
    } else {
      localTree *C;
      if (Cu->getLevel() >= Cv->getLevel())
        C = new localTree(Cu, Cv);
      else
        C = new localTree(Cv, Cu);
      l = C->getLevel();
    }
  } else {
    auto lca = *iu;
    while (*iu == *iv) {
      lca = *iu;
      iu++;
      iv++;
    }
    Cu = *iu;
    Cv = *iv;
    l = lca->getLevel();
  }

  if (this->blocked_insert) {
    localTree *p = localTree::getParent(Cu);
    while (Cu->getSize() + Cv->getSize() <= (1 << (p->getLevel() - 1))) {
      assert(Cu != Cv);
      assert(localTree::getParent(Cu) == p && localTree::getParent(Cv) == p);
      if (Cu->getLevel() == p->getLevel() - 1 || Cv->getLevel() == p->getLevel() - 1) {
        if (Cu->getLevel() == Cv->getLevel()) { // Merge two nodes directly under parent
          localTree::deleteFromParent(Cu);
          localTree::deleteFromParent(Cv);
          localTree::merge(Cu, Cv);
          if (p->getSize() == 0) {
            auto gp = localTree::getParent(p);
            localTree::deleteFromParent(p);
            localTree::addChild(gp, Cu);
            delete p;
          } else localTree::addChild(p, Cu);
          iu++;
          iv++;
          Cu = *iu;
          Cv = *iv;
        } else if (Cu->getLevel() > Cv->getLevel()) { // If one node directly under parent add other as child
          if (Cu->getSize() + Cv->getSize() > (1 << Cu->getLevel()))
            break;
          localTree::deleteFromParent(Cu);
          localTree::deleteFromParent(Cv);
          localTree::addChild(Cu, Cv);
          localTree::addChild(p, Cu);
          iu++;
          Cu = *iu;
        } else { // If one node directly under parent add other as child
          if (Cv->getSize() + Cu->getSize() > (1 << Cv->getLevel())) break;
          localTree::deleteFromParent(Cu);
          localTree::deleteFromParent(Cv);
          localTree::addChild(Cv, Cu);
          localTree::addChild(p, Cv);
          iv++;
          Cv = *iv;
        }
      } else { // For both nodes far below parent create new parent as far down as possible
        localTree::deleteFromParent(Cu);
        localTree::deleteFromParent(Cv);
        localTree *C;
        if (Cu->getLevel() >= Cv->getLevel()) C = new localTree(Cu, Cv);
        else C = new localTree(Cv, Cu);
        C->setLevel(std::max(std::max(Cu->getLevel(),Cv->getLevel())+1, (size_t)std::ceil(std::log2(C->getSize()))));
        localTree::addChild(p, C);
      }
      if (iu == pu.rend() || iv == pv.rend()) break;
      p = localTree::getParent(Cu);
    }
    auto lu = 1;
    auto lv = 1;
    if (iu != pu.rend()) lu = Cu->getLevel();
    if (iv != pv.rend()) lv = Cv->getLevel();
    l = std::max(lu, lv) + 1;
  }

  leaves[u]->insertToLeaf(v, l);
  leaves[v]->insertToLeaf(u, l);
}
inline void SCCWN::remove(size_t u, size_t v) {
  // std::cout << u << " " << v << std::endl;
  assert(leaves[u]->getEdgeLevel(v) == leaves[v]->getEdgeLevel(u));
  size_t l = leaves[u]->getEdgeLevel(v);
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
          Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(), nCv,
                      l);
          Rstat.push_back(std::move(info));
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

        Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(), nCv, l);
        Rstat.push_back(std::move(info));

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

          Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(), nCv,
                      l);
          Rstat.push_back(std::move(info));
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

        Radius info(1, Ru.size(), Eu.size(), nCu, Rv.size(), Ev.size(), nCv, l);
        Rstat.push_back(std::move(info));

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
      std::ofstream fout;
      if (stat)
        fout.open(filepath + "/" + std::to_string(i) + ".txt");
      parlay::sequence<stats> info;
      localTree::traverseTopDown(roots[i], clear, verbose, stat, info);
      if (stat) {
        parlay::sort_inplace(
            info, [&](stats x, stats y) { return x.level > y.level; });
        for (auto it : info)
          fout << it.level << " " << it.fanout << " " << it.height << " "
               << it.size << std::endl;
        fout.close();
      }
    }
  });
  if (stat)
    std::cout << "quiet memory usage is " << stats::memUsage << " bytes\n";
  if (stat) {
    parlay::sort_inplace(Rstat, [&](const Radius &a, const Radius &b) {
      return a.level > b.level;
    });
    std::ofstream fradius;
    fradius.open(filepath + ".rad");
    for (auto it : Rstat)
      fradius << it.found << " " << it.nRu << " " << it.nEu << " " << it.nCu
              << " " << it.nRv << " " << it.nEv << " " << it.nCv << " "
              << it.level << std::endl;
    fradius.close();
  }
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
