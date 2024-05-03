#include "cluster_graph.hpp"
#include <parlay/alloc.h>
#include <fstream>
class blocked_arr {
 public:
  parlay::sequence<cluster_graph *> leaves;
  blocked_arr(size_t n) : leaves(parlay::sequence<cluster_graph *>(n, nullptr)){};
  void insert(size_t u, size_t v);

  bool is_connected(size_t u, size_t v);

  void run_stat(std::string filepath = "./", bool verbose = false) { statistic(filepath, false, true, verbose); }
  static void printCG(parlay::sequence<cluster_graph *> &path);
  void statistic(std::string filepath, bool clear, bool stat, bool verbose);
  void checkEdgeBlocked(size_t u, size_t v);
  ~blocked_arr() { statistic("./", true, false, false); }

 private:
  struct pathblock {
    char x[960];
  };
  using pathAllocator = parlay::type_allocator<pathblock>;
  static cluster_graph **alloc() { return (cluster_graph **)pathAllocator::alloc(); }
  static void dealloc(cluster_graph **p) { pathAllocator::free((pathblock *)p); }
  static void updateToTop(cluster_graph *src, cluster_graph *dest, size_t inc);
  static size_t pushDown(cluster_graph **path, size_t Len, cluster_graph *v);
  static size_t compressPath(cluster_graph *cu, cluster_graph *cv, cluster_graph *cp);
  void insertEdge(size_t u, size_t v, size_t l);
  static cluster_graph *merge(cluster_graph **pu, cluster_graph **pv, size_t uLen, size_t vLen);
};
inline void blocked_arr::insert(size_t u, size_t v) {
  // std::cout << "================================" << std::endl;
  cluster_graph **pu = alloc();
  cluster_graph **pv = alloc();
  size_t uLen = 0, vLen = 0;
  auto g = [&](size_t x, cluster_graph **p) -> size_t {
    // std::cout << x << p << std::endl;
    if (leaves[x] == nullptr) {
      leaves[x] = new cluster_graph(0, x, 1);  // level 0 with size 1
    }
    return cluster_graph::getRootPathLen(leaves[x], p);  // get the length of path to root
  };

  parlay::par_do([&]() { uLen = g(u, pu); }, [&]() { vLen = g(v, pv); });
  auto *rootu = pu[uLen - 1];
  auto *rootv = pv[vLen - 1];
  // cluster_graph::cleanTopDown(rootu, false, nullptr, false, true);
  // std::cout << leaves[u] << "~~~~~~~~~~~~~~~~~~" << leaves[v] << std::endl;
  // cluster_graph::cleanTopDown(rootv, false, nullptr, false, true);
  if (cluster_graph::getLevel(rootu) < cluster_graph::getLevel(rootv)) {
    std::swap(u, v);
    std::swap(pu, pv);
    std::swap(uLen, vLen);
    std::swap(rootu, rootv);
  }
  if (rootu != rootv) {
    if (cluster_graph::isBlocked(rootu, rootv, cluster_graph::getLevel(rootu))) {
      // std::cout << "direct connect\n";
      ASSERT_MSG(cluster_graph::getLevel(rootu) >= cluster_graph::getLevel(rootv), "direct connect");
      auto np = cluster_graph::createFromTwo(rootu, rootv);
      ASSERT_MSG(cluster_graph::getSize(np) <= (1 << cluster_graph::getLevel(np)), "create np size wrong");
      insertEdge(u, v, cluster_graph::getLevel(np));
      dealloc(pu);
      dealloc(pv);
      return;
    }
    size_t incre = cluster_graph::getSize(rootv);
    ASSERT_MSG(!cluster_graph::isBlocked(pu[uLen - 1], pv[vLen - 1], cluster_graph::getLevel(pu[uLen - 1])),
               "pu pv expected to be unblocked at root");
    auto np = merge(pu, pv, uLen, vLen);
    ASSERT_MSG(cluster_graph::getSize(np) <= (1 << cluster_graph::getLevel(np)), "merge np size wrong");
    // updateToTop(cluster_graph::getParent(np), cluster_graph::getRoot(np), incre);
    insertEdge(u, v, cluster_graph::getLevel(np));
  } else {
    cluster_graph *lca = nullptr;
    while (pu[uLen - 1] == pv[vLen - 1]) {
      lca = pu[uLen - 1];
      uLen--;
      vLen--;
    }
    if (cluster_graph::getLevel(pu[uLen - 1]) < cluster_graph::getLevel(pv[vLen - 1])) {
      std::swap(pu, pv);
      std::swap(u, v);
      std::swap(uLen, vLen);
    }
    if (cluster_graph::isBlocked(pu[uLen - 1], pv[vLen - 1], cluster_graph::getLevel(pu[uLen - 1]))) {
      if (cluster_graph::getLevel(lca) - cluster_graph::getLevel(pu[uLen - 1]) > 1) {
        ASSERT_MSG(cluster_graph::getSize(lca) <= (1 << cluster_graph::getLevel(lca)), "lca size wrong");
        cluster_graph::cutChild(pu[uLen - 1], lca);
        cluster_graph::cutChild(pv[vLen - 1], lca);
        auto np = cluster_graph::createFromTwo(pu[uLen - 1], pv[vLen - 1]);
        ASSERT_MSG(cluster_graph::getLevel(lca) > cluster_graph::getLevel(np), "level np cannot be the child of lca");
        ASSERT_MSG(!cluster_graph::isBlocked(lca, np, cluster_graph::getLevel(lca)), "np cannot be the child of lca");
        cluster_graph::addChild(lca, np);
        insertEdge(u, v, cluster_graph::getLevel(np));
      } else {  // already blocked
        insertEdge(u, v, cluster_graph::getLevel(lca));
      }

      dealloc(pu);
      dealloc(pv);
      return;
    }
    cluster_graph::cutChild(pv[vLen - 1], lca);
    size_t incre = cluster_graph::getSize(pv[vLen - 1]);
    // pu[uLen] = lca;
    // pu[uLen+1] should be lca
    // we cut subtree rooted at pv[vLen-1] from subtree rooted at lca and reinsert to block level
    ASSERT_MSG(pu[uLen] == lca, "pu[uLen] != lca");
    ASSERT_MSG(pv[vLen] == lca, "pv[vLen] != lca");
    ASSERT_MSG(!cluster_graph::isBlocked(pu[uLen - 1], pv[vLen - 1], cluster_graph::getLevel(pu[uLen - 1])),
               "pu pv expected to be unblocked at lca uLen+1");
    auto np = merge(pu, pv, uLen + 1, vLen);
    ASSERT_MSG(cluster_graph::getSize(np) <= (1 << cluster_graph::getLevel(np)), "mrege np from pu pv size wrong");
    // updateToTop(cluster_graph::getParent(np), lca, incre);
    insertEdge(u, v, cluster_graph::getLevel(np));
  }
  dealloc(pu);
  dealloc(pv);
}
inline cluster_graph *blocked_arr::merge(cluster_graph **pu, cluster_graph **pv, size_t uLen, size_t vLen) {
  // if l(rootu) == l(rootv) they have to be blocked which should not in this function
  ASSERT_MSG(!cluster_graph::isBlocked(pu[uLen - 1], pv[vLen - 1], cluster_graph::getLevel(pu[uLen - 1])),
             "Tu Tv expected to be unblocked at root");
  // if unblocked, l(rootu) > l(rootv)
  // 2^i < n(rootu) <= 2^i+1 if l(rootv) == l(rootu) they must blocked
  ASSERT_MSG(cluster_graph::getLevel(pu[uLen - 1]) > cluster_graph::getLevel(pv[vLen - 1]), "lrootu<=lootv");
  // if (cluster_graph::getLevel(pu[uLen - 1]) < cluster_graph::getLevel(pv[vLen - 1])) {
  //   std::swap(pu, pv);
  //   std::swap(uLen, vLen);
  // }
  auto root = pu[uLen - 1];
  auto cv = pv[vLen - 1];
  size_t i = pushDown(pu, uLen, cv);
  auto cu = pu[i];
  auto cp = pu[i + 1];
  // cv added as cp's child
  // level difference 1 no compress
  if (cluster_graph::getLevel(cp) - cluster_graph::getLevel(cu) == 1) {
    ASSERT_MSG(cluster_graph::getLevel(cp) > cluster_graph::getLevel(cv), "cv cannot be a child of cp");
    ASSERT_MSG(!cluster_graph::isBlocked(cp, cv, cluster_graph::getLevel(cp)), "cv cannot be a child of cp");
    cluster_graph::addChild(cp, cv);
    ASSERT_MSG(cluster_graph::getSize(cp) <= (1 << cluster_graph::getLevel(cp)), "merge 1 wrong");
    auto np = cp;
    updateToTop(cp, root, cluster_graph::getSize(cv));
    return np;
  }
  // cu cv form a new node to compress path
  // level difference > 1 need to compress
  if (cluster_graph::getLevel(cu) >= cluster_graph::getLevel(cv)) {
    cluster_graph::cutChild(cu, cp);
    auto np = cluster_graph::createFromTwo(cu, cv);
    ASSERT_MSG(cluster_graph::getSize(np) <= (1 << cluster_graph::getLevel(np)), "merge 2 cannot create");
    ASSERT_MSG(cluster_graph::isBlocked(cu, cv, cluster_graph::getLevel(cu)), "not in merge 2");
    ASSERT_MSG(cluster_graph::getLevel(cp) > cluster_graph::getLevel(np), "np cannot be the child of cp");
    ASSERT_MSG(!cluster_graph::isBlocked(cp, np, cluster_graph::getLevel(cp)), "np cannot be the child of cp");
    cluster_graph::addChild(cp, np);
    updateToTop(cp, root, cluster_graph::getSize(cv));
    return np;
  }
  // l(cv)>l(cu) cut cu, insert cv as child of cp, reinsert cv;
  cluster_graph::cutChild(cu, cp);
  ASSERT_MSG(cluster_graph::getLevel(cp) > cluster_graph::getLevel(cv), "cv cannot be the child of cp");
  ASSERT_MSG(!cluster_graph::isBlocked(cp, cv, cluster_graph::getLevel(cp)), "cv cannot be the child of cp");
  cluster_graph::addChild(cp, cv);
  auto incre = cluster_graph::getSize(cv);
  pv[vLen++] = cp;
  auto np = merge(pv, pu, vLen, i + 1);
  updateToTop(cp, root, incre);
  return np;
}
inline void blocked_arr::printCG(parlay::sequence<cluster_graph *> &path) {
  std::copy(path.begin(), path.end(), std::ostream_iterator<cluster_graph *>(std::cout, ","));
  std::cout << "\n\n";
}
inline void blocked_arr::statistic(std::string filepath = "./", bool clear = false, bool stat = false,
                                   bool verbose = false) {
  parlay::sequence<cluster_graph *> roots(leaves.size(), nullptr);
  parlay::parallel_for(0, leaves.size(), [&](size_t i) {
    if (leaves[i]) roots[i] = cluster_graph::getRoot(leaves[i]);
  });
  if (verbose) printCG(leaves);
  if (verbose) printCG(roots);
  roots = parlay::remove_duplicates(roots);
  if (verbose) printCG(roots);
  parlay::parallel_for(0, roots.size(), [&](size_t i) {
    if (roots[i]) {
      std::ofstream fout;
      if (stat) fout.open(filepath + "/" + std::to_string(i) + ".txt");
      parlay::sequence<stats> st;
      cluster_graph::cleanTopDown(roots[i], clear, st, stat, verbose);
      if (stat) {
        parlay::sort_inplace(st, [&](stats x, stats y) { return x.level > y.level; });
        for (auto it : st)
          fout << it.level << " " << it.fanout << " " << " " << it.size << std::endl;
        fout.close();
      }
    }
  });
  if (stat) std::cout << "quiet memory usage is " << stats::memUsage << " bytes\n";
}
inline bool blocked_arr::is_connected(size_t u, size_t v) {
  ASSERT_MSG(leaves[u] != nullptr, "no such vertex");
  ASSERT_MSG(leaves[v] != nullptr, "no such vertex");
  return cluster_graph::getRoot(leaves[u]) == cluster_graph::getRoot(leaves[v]);
}
inline size_t blocked_arr::pushDown(cluster_graph **path, size_t Len, cluster_graph *v) {
  ASSERT_MSG(cluster_graph::getLevel(path[Len - 1]) >= cluster_graph::getLevel(v), "u is not belong to higher tree");
  size_t i = Len - 1;
  while (!cluster_graph::isBlocked(path[i], v, cluster_graph::getLevel(path[i]))) {
    ASSERT_MSG(i >= 0, "push down to wrong level");
    i--;
  }
  ASSERT_MSG(!cluster_graph::isBlocked(path[i + 1], v, cluster_graph::getLevel(path[i + 1])),
             "(path[i+1],v) expected be unblocked");
  ASSERT_MSG(cluster_graph::isBlocked(path[i], v, cluster_graph::getLevel(path[i])),
             "(path[i],v) expected to be blocked");
  return i;
}
inline void blocked_arr::insertEdge(size_t u, size_t v, size_t l) {
  leaves[u]->insertToLeaf(v, l);
  leaves[v]->insertToLeaf(u, l);
}
inline void blocked_arr::updateToTop(cluster_graph *src, cluster_graph *dest, size_t incre) {
  ASSERT_MSG(dest != nullptr, "dest wrong");
  auto p = src;
  while (p != nullptr) {
    if (p != src) {  // src all set
      cluster_graph::increSize(p, incre);
      if (cluster_graph::getSize(p) > (1 << cluster_graph::getLevel(p))) {
        std::cout << p << "," << src << "," << dest << "," << incre << std::endl;
        parlay::sequence<stats> tmp;
        cluster_graph::cleanTopDown(cluster_graph::getRoot(p), false, tmp, false, true);
      }
      ASSERT_MSG(cluster_graph::getSize(p) <= (1 << cluster_graph::getLevel(p)), "cannot incre");
      cluster_graph::sortChild(p);
    }
    if (p != dest)
      cluster_graph::increParentChildSize(p, incre);
    else
      return;
    p = cluster_graph::getParent(p);
  }
}
inline void blocked_arr::checkEdgeBlocked(size_t u, size_t v) {
  ASSERT_MSG(cluster_graph::getEdgeLevel(leaves[u], v) == cluster_graph::getEdgeLevel(leaves[v], u),
             "edge inserted to diff level");
  auto Cu = cluster_graph::getAncestor(leaves[u], cluster_graph::getEdgeLevel(leaves[u], v));
  auto Cv = cluster_graph::getAncestor(leaves[v], cluster_graph::getEdgeLevel(leaves[v], u));
  ASSERT_MSG(cluster_graph::getParent(Cu) == cluster_graph::getParent(Cv), "CuCv not in same cluster graph");
  ASSERT_MSG(cluster_graph::isBlocked(Cu, Cv, std::max(cluster_graph::getLevel(Cu), cluster_graph::getLevel(Cv))),
             "expect to be blocked");
}