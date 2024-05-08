#include "localTree.hpp"
#include <fstream>
#include <queue>
#include <unordered_set>
class CWN {
 private:
  static std::tuple<bool, size_t, size_t> fetchEdge(std::queue<localTree *> &Q, size_t l);
  void placeEdges(parlay::sequence<std::pair<size_t, size_t>> &edges, size_t l);
  void printNodes(parlay::sequence<localTree *> &Nodes) {
    std::copy(Nodes.begin(), Nodes.end(), std::ostream_iterator<localTree *>(std::cout, ","));
    std::cout << "\n\n";
  }
  void printSearch(size_t found, size_t nRu, size_t nEu, size_t nRv, size_t nEv, size_t level) {
    std::cout << found << " " << nRu << " " << nEu << " " << nRv << " " << nEv << " " << level << std::endl;
  }

 public:
  size_t n;
  static size_t lmax;
  static bool verbose;
  parlay::sequence<localTree *> leaves;
  CWN(size_t _n) : n(_n), leaves(parlay::sequence<localTree *>(n, nullptr)) {}
  ~CWN() { run_stat("./", false, true, false); };
  void insert(size_t u, size_t v);
  bool is_connected(size_t u, size_t v);
  void remove(size_t u, size_t v);
  void run_stat(std::string filepath, bool verbose, bool clear, bool stat);
  int64_t space();
};
inline size_t CWN::lmax = 63;
// 0/1 num Ru,Eu,Rv,Ev,CP->getLevel()
inline bool CWN::verbose = false;

int64_t CWN::space() {
  int64_t space = 0;
  space += sizeof(std::vector<localTree*>);
  space += leaves.size() * sizeof(localTree*);
  space += 2*sizeof(size_t);
  std::set<rankTree*> visited_rnodes;
  std::set<localTree*> visited_lnodes;
  for (localTree* leaf : leaves) {
    if (leaf == nullptr) continue;
    bool continueloop = false;
    space += leaf->space();
    space += leaf->vertex->space();
    rankTree* rTree = leaf->parent;
    if (!rTree) continue;
    visited_rnodes.insert(rTree);
    while (rTree->parent) {
      rTree = rTree->parent;
      if (visited_rnodes.find(rTree) != visited_rnodes.end()) {
        continueloop = true;
        break;
      }
      visited_rnodes.insert(rTree);
    }
    if (continueloop) continue;
    localTree* lTree = rTree->Node;
    if (visited_lnodes.find(lTree) != visited_lnodes.end()) continue;
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
      if (continueloop) break;
      lTree = rTree->Node;
      if (visited_lnodes.find(lTree) != visited_lnodes.end()) break;
      visited_lnodes.insert(lTree);
    }
  }
  space += visited_rnodes.size() * sizeof(rankTree);
  for (localTree* lTree : visited_lnodes)
    space += lTree->space();
  return space;
}

inline void CWN::insert(size_t u, size_t v) {
  auto g = [](size_t &u) -> localTree * {
    localTree *r = new localTree(u);
    localTree *l = r;
    while (r->getLevel() < lmax) {
      auto p = new localTree();
      p->setLevel(r->getLevel() + 1);
      localTree::addChild(p, r);
      r = p;
    }
    return l;
  };
  if (leaves[u] == nullptr) leaves[u] = g(u);
  if (leaves[v] == nullptr) leaves[v] = g(v);

  auto Cu = localTree::getRoot(leaves[u]);
  auto Cv = localTree::getRoot(leaves[v]);
  if (Cu != Cv) localTree::merge(Cu, Cv);
  leaves[u]->insertToLeaf(v, lmax);
  leaves[v]->insertToLeaf(u, lmax);
}

inline void CWN::remove(size_t u, size_t v) {
  // std::cout << u << " " << v << std::endl;
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

  std::queue<localTree *> Qu, Qv;                      // ready to fetch
  parlay::sequence<std::pair<size_t, size_t>> Eu, Ev;  // fetched edge
  std::unordered_set<localTree *> Hu, Hv;              // visited node
  parlay::sequence<localTree *> Ru, Rv;                // visited node
  auto init = [](std::queue<localTree *> &Q, parlay::sequence<std::pair<size_t, size_t>> &E,
                 parlay::sequence<localTree *> &R, std::unordered_set<localTree *> &HT, localTree *C) -> void {
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
    if (CP) assert(CP->getLevel() == l);
    while (true) {
      auto eu = fetchEdge(Qu, l);
      if (std::get<0>(eu) == true) {
        auto Cuv = localTree::getLevelNode(leaves[std::get<2>(eu)], l);
        assert(Cuv != nullptr);
        assert(CP == nullptr || localTree::getParent(Cuv) == CP);
        assert(Cuv->getLevel() == l - 1);
        if (Hv.find(Cuv) != Hv.end()) {
          if (Eu.empty()) {
            // don't push nCu+nCv may violate size
          } else if (nCu <= nCv) {
            for (auto it : Ru)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            C->setLevel(l - 1);
            for (auto it : Ru) {
              assert(it->getLevel() == l - 1);
              localTree::merge(C, it);
            }
            localTree::addChild(CP, C);
            placeEdges(Eu, l - 1);
            placeEdges(Ev, l);
          } else {
            for (auto it : Rv)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            C->setLevel(l - 1);
            for (auto it : Rv) {
              assert(it->getLevel() == l - 1);
              localTree::merge(C, it);
            }
            assert(CP->getLevel() - C->getLevel() == 1);
            localTree::addChild(CP, C);
            placeEdges(Eu, l);
            placeEdges(Ev, l - 1);
          }
          if (verbose) printSearch(1, Ru.size(), Eu.size(), Rv.size(), Ev.size(), l);
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
        localTree *_CP = new localTree();
        _CP->setLevel(l);
        if (Eu.empty()) {
          localTree::deleteFromParent(Ru[0]);
          localTree::addChild(_CP, Ru[0]);
        } else if (nCu <= nCv) {
          for (auto it : Ru)
            localTree::deleteFromParent(it);
          auto C = new localTree();
          C->setLevel(l - 1);
          for (auto it : Ru) {
            assert(it->getLevel() == l - 1);
            localTree::merge(C, it);
          }
          localTree::addChild(_CP, C);
          placeEdges(Eu, l - 1);
          placeEdges(Ev, l);
        } else {
          for (auto it : Ru)
            localTree::deleteFromParent(it);
          for (auto it : Ru)
            localTree::addChild(_CP, it);
          for (auto it : Rv)
            localTree::deleteFromParent(it);
          auto C = new localTree();
          C->setLevel(l - 1);
          for (auto it : Rv) {
            assert(it->getLevel() == l - 1);
            localTree::merge(C, it);
          }
          localTree::addChild(CP, C);
          placeEdges(Eu, l);
          placeEdges(Ev, l - 1);
        }
        localTree::add2Children(GP, CP, _CP);
        Cu = _CP;
        Cv = CP;
        CP = GP;
        l = CP ? l + 1 : 0;
        if (verbose) printSearch(0, Ru.size(), Eu.size(), Rv.size(), Ev.size(), l);
        break;
      }
      auto ev = fetchEdge(Qv, l);
      if (std::get<0>(ev) == true) {
        auto Cuv = localTree::getLevelNode(leaves[std::get<2>(ev)], l);
        assert(Cuv != nullptr);
        assert(CP == nullptr || localTree::getParent(Cuv) == CP);
        assert(Cuv->getLevel() == l - 1);
        if (Hu.find(Cuv) != Hu.end()) {
          if (Ev.empty()) {
            placeEdges(Eu, l);
          } else if (nCu <= nCv) {
            for (auto it : Ru)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            C->setLevel(l - 1);
            for (auto it : Ru) {
              assert(it->getLevel() == l - 1);
              localTree::merge(C, it);
            }
            assert(CP->getLevel() - C->getLevel() == 1);
            localTree::addChild(CP, C);
            placeEdges(Eu, l - 1);
            placeEdges(Ev, l);
          } else {
            for (auto it : Rv)
              localTree::deleteFromParent(it);
            auto C = new localTree();
            C->setLevel(l - 1);
            for (auto it : Rv) {
              assert(it->getLevel() == l - 1);
              localTree::merge(C, it);
            }
            assert(CP->getLevel() - C->getLevel() == 1);
            localTree::addChild(CP, C);
            placeEdges(Eu, l);
            placeEdges(Ev, l - 1);
          }
          if (verbose) printSearch(1, Ru.size(), Eu.size(), Rv.size(), Ev.size(), l);
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
        localTree *_CP = new localTree();
        _CP->setLevel(l);
        if (Ev.empty()) {
          localTree::deleteFromParent(Rv[0]);
          localTree::addChild(_CP, Rv[0]);
          placeEdges(Eu, l);
        } else if (nCv <= nCu) {
          for (auto it : Rv)
            localTree::deleteFromParent(it);
          auto C = new localTree();
          C->setLevel(l - 1);
          for (auto it : Rv) {
            assert(it->getLevel() == l - 1);
            localTree::merge(C, it);
          }
          localTree::addChild(_CP, C);
          placeEdges(Eu, l);
          placeEdges(Ev, l - 1);
        } else {
          for (auto it : Rv)
            localTree::deleteFromParent(it);
          for (auto it : Rv) {
            assert(it->getLevel() == l - 1);
            localTree::addChild(_CP, it);
          }
          for (auto it : Ru)
            localTree::deleteFromParent(it);
          auto C = new localTree();
          C->setLevel(l - 1);
          for (auto it : Ru) {
            assert(it->getLevel() == l - 1);
            localTree::merge(C, it);
          }
          localTree::addChild(CP, C);
          placeEdges(Eu, l - 1);
          placeEdges(Ev, l);
        }
        localTree::add2Children(GP, CP, _CP);
        Cu = CP;
        Cv = _CP;
        CP = GP;
        l = CP ? l + 1 : 0;
        if (verbose) printSearch(0, Ru.size(), Eu.size(), Rv.size(), Ev.size(), l);
        break;
      }
    }
  }
}
inline bool CWN::is_connected(size_t u, size_t v) {
  return localTree::getRoot(leaves[u]) == localTree::getRoot(leaves[v]);
}
inline void CWN::run_stat(std::string filepath, bool verbose = false, bool clear = false, bool stat = true) {
  parlay::sequence<localTree *> roots(leaves.size(), nullptr);
  parlay::sequence<localTree *> parents(leaves.size(), nullptr);
  parlay::parallel_for(0, roots.size(), [&](size_t i) {
    if (leaves[i]) roots[i] = localTree::getRoot(leaves[i]);
    if (leaves[i]) parents[i] = localTree::getParent(leaves[i]);
  });
  stats::memUsage = 0;
  if (verbose) printNodes(leaves);
  if (verbose) printNodes(parents);
  if (verbose) printNodes(roots);
  roots = parlay::remove_duplicates(roots);
  if (verbose) printNodes(roots);
  parlay::parallel_for(0, roots.size(), [&](size_t i) {
    if (roots[i]) {
      std::ofstream fout;
      if (stat) fout.open(filepath + "/" + std::to_string(i) + ".txt");
      parlay::sequence<stats> info;
      localTree::traverseTopDown(roots[i], clear, verbose, stat, info);
      if (stat) {
        parlay::sort_inplace(info, [&](stats x, stats y) { return x.level > y.level; });
        for (auto it : info)
          fout << it.level << " " << it.fanout << " " << it.height << " " << it.size << std::endl;
        fout.close();
      }
    }
  });
  if (stat) std::cout << "quiet memory usage is " << stats::memUsage << " bytes\n";
}
inline void CWN::placeEdges(parlay::sequence<std::pair<size_t, size_t>> &edges, size_t l) {
  for (auto it : edges) {
    leaves[it.first]->insertToLeaf(it.second, l);
    leaves[it.second]->insertToLeaf(it.first, l);
  }
}
inline std::tuple<bool, size_t, size_t> CWN::fetchEdge(std::queue<localTree *> &Q, size_t l) {
  auto node = Q.front();
  auto e = localTree::fetchEdge(node, l);
  while (!std::get<0>(e)) {
    Q.pop();
    if (Q.empty()) return std::make_tuple(false, 0, 0);
    node = Q.front();
    e = localTree::fetchEdge(node, l);
  }
  return e;
}