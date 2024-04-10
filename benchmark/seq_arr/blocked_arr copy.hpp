#include "cluster_graph.hpp"

class blocked_arr {
 public:
  parlay::sequence<cluster_graph *> leaves;
  blocked_arr(size_t n) : leaves(parlay::sequence<cluster_graph *>(n, nullptr)){};
  void insert(size_t u, size_t v);
  void insertEdge(size_t u, size_t v, size_t l);
  bool is_connected(size_t u, size_t v);
  void statistic(bool clear, bool stat, bool verbose);
  void run_stat() { statistic(false, true, false); }
  static void printCG(parlay::sequence<cluster_graph *> &path);
  static void updateToTop(cluster_graph *src, cluster_graph *dest, size_t inc);
  static cluster_graph *pushDown(parlay::sequence<cluster_graph *> &p, cluster_graph *v);
  static size_t compressPath(cluster_graph *cu, cluster_graph *cv, cluster_graph *cp);
  ~blocked_arr() { statistic(true, true, false); }
};
inline void blocked_arr::insert(size_t u, size_t v) {
  auto g = [&](size_t x) {
    if (leaves[x] == nullptr) {
      leaves[x] = new cluster_graph(0, x, 1);  // level 0 with size 1
    }
    parlay::sequence<cluster_graph *> path;
    cluster_graph::getRootPath(leaves[x], path);  // get path from leaf to root
    return path;
  };
  parlay::sequence<cluster_graph *> pu, pv;
  parlay::par_do([&]() { pu = g(u); }, [&]() { pv = g(v); });
  if (cluster_graph::getLevel(*pu.rbegin()) < cluster_graph::getLevel(*pv.rbegin())) {
    // make sure u is the one with higher or equal level
    std::swap(u, v);
    std::swap(pu, pv);
  }
  auto iu = pu.rbegin();
  auto iv = pv.rbegin();
  // std::cout << "&Cu = " << *iu << " &Cv = " << *iv << " parent = " << parent << std::endl;
  if (*iu != *iv) {  // not connected
    if (cluster_graph::isBlocked(*iu, *iv, cluster_graph::getLevel(*iu))) {
      // case 1 2
      // insert edge (u,v) at level parent->level
      auto parent = cluster_graph::createFromTwo(*iu, *iv);
      insertEdge(u, v, cluster_graph::getLevel(*iu) + 1);
    } else {
      // start from the leaf of u, go upper until meet a unblocked node, insert v as child
      // std::cout << "not connect case 3 4\n";
      // std::cout << "45\n";
      // std::cout << " &pu = " << *iu << " with level " << (*iu)->getLevel() << " &v = " << *iv << " with level "
      //           << (*iv)->getLevel() << std::endl;
      // if no compression, then add n(v) to the size of parent,update to root
      // if compression, delete u from p
      // parent of *iv has correct size and ch->size;
      auto cv = *iv;
      auto cu = pushDown(pu, cv);
      auto cp = cluster_graph::getParent(cu);
      if (cluster_graph::getLevel(cp) - cluster_graph::getLevel(cu) == 1) {
        cluster_graph::addChild(cp, cv);
        insertEdge(u, v, cluster_graph::getLevel(cp));
      } else {
        auto incre = cluster_graph::getSize(cv);
        auto level = compressPath(cu, cv, cp);
        // update to root *(pu.rbegin())
        updateToTop(cp, *iu, incre);
        insertEdge(u, v, level);
      }
    }
  } else {  // u,v  connected
    // find lca(u,v) and put it into parent
    // std::cout << " &u = " << *iu << " &v = " << *iv << " &parent = " << parent << std::endl;
    cluster_graph *lca = nullptr;
    while (*iu == *iv) {
      lca = *iu;
      iu++;
      iv++;
    }
    // std::cout << " &u = " << *iu << " with level " << (*iu)->getLevel() << " &v = " << *iv << " with level "
    //           << (*iv)->getLevel() << " &parent = " << parent << " with level " << parent->getLevel() << std::endl;
    if (cluster_graph::getLevel(*iu) < cluster_graph::getLevel(*iv)) {
      // we want to *iu to be higher level so we cut *iv
      std::swap(u, v);
      std::swap(pu, pv);
      std::swap(iu, iv);
    }
    // std::cout << " &u = " << *iu << " with level " << (*iu)->getLevel() << " &v = " << *iv << " with level "
    //           << (*iv)->getLevel() << " &parent = " << parent << " with level " << parent->getLevel() << std::endl;

    if (cluster_graph::isBlocked(*iu, *iv, cluster_graph::getLevel(*iu))) {
      // blocked edge goes up
      if (cluster_graph::getLevel(lca) - cluster_graph::getLevel(*iu) == 1) {
        // no compression
        // std::cout << "connected no compress\n";
        insertEdge(u, v, cluster_graph::getLevel(lca));
      } else {
        // compression
        // delete *iu, create new tree from iu iv and plug into parent's children
        cluster_graph::cutChild(*iv, lca);
        cluster_graph::decreSize(lca, cluster_graph::getSize(*iv));
        auto cv = *iv;
        auto cu = pushDown(pu, cv);
        auto cp = cluster_graph::getParent(cu);
        auto incre = cluster_graph::getSize(cv);
        auto level = compressPath(cu, cv, cp);
        updateToTop(cp, *iu, incre);
        insertEdge(u, v, level);
        // auto l = cluster_graph::compressPath(*iu, *iv, lca);
        // leaves[u]->insertToLeaf(v, l);
        // leaves[v]->insertToLeaf(u, l);
        // auto src = cluster_graph::getParent(cluster_graph::getParent(*iv));
        // if (src) cluster_graph::updateToTop(src, parent, cluster_graph::getSize(*iv));
        // update to lca parent
      }
    } else {
      // unblocked edge goes down
      //  delete *iv, link to the path of *iu
      // cut from leaf to *iu;
      //  p = lca(u,v)
      //  std::cout << "connected compress\n";
      // u v not block
      cluster_graph::cutChild(*iv, parent);
      std::cout << "97\n";
      auto l = cluster_graph::pushDownToBeChild(pu, *iv, parent);
      auto src = cluster_graph::getParent(cluster_graph::getParent(*iv));
      if (src) cluster_graph::updateToTop(src, parent, cluster_graph::getSize(*iv));
      leaves[u]->insertToLeaf(v, l);
      leaves[v]->insertToLeaf(u, l);
      // update to lca parent
    }
  }
}
inline void blocked_arr::printCG(parlay::sequence<cluster_graph *> &path) {
  std::copy(path.begin(), path.end(), std::ostream_iterator<cluster_graph *>(std::cout, ","));
  std::cout << "\n\n";
}
inline void blocked_arr::statistic(bool clear, bool stat, bool verbose) {
  parlay::sequence<cluster_graph *> roots(leaves.size(), nullptr);
  parlay::parallel_for(0, leaves.size(), [&](size_t i) {
    if (leaves[i]) roots[i] = cluster_graph::getRoot(leaves[i]);
  });
  if (verbose) printCG(leaves);
  if (verbose) printCG(roots);
  roots = parlay::remove_duplicates(roots);
  if (verbose) printCG(roots);
  parlay::sequence<cluster_graph::stat *> st(roots.size());
  parlay::parallel_for(0, roots.size(), [&](size_t i) { st[i] = new cluster_graph::stat(); });
  // parlay::parallel_for(0, roots.size(), [&](size_t i) {
  //   if (roots[i]) cluster_graph::cleanTopDown(roots[i], clear, st[i], stat, verbose);
  // });
  for (size_t i = 0; i < roots.size(); i++)
    if (roots[i]) cluster_graph::cleanTopDown(roots[i], clear, st[i], stat, verbose);
  if (stat) {
    for (size_t i = 0; i < st.size(); i++)
      if (!st[i]->info.empty()) st[i]->printStat(i);
  }
  parlay::parallel_for(0, roots.size(), [&](size_t i) {
    st[i]->info.clear();
    delete st[i];
  });
}
inline bool blocked_arr::is_connected(size_t u, size_t v) {
  ASSERT_MSG(leaves[u] != nullptr, "no such vertex");
  ASSERT_MSG(leaves[v] != nullptr, "no such vertex");
  return cluster_graph::getRoot(leaves[u]) == cluster_graph::getRoot(leaves[v]);
}
inline cluster_graph *blocked_arr::pushDown(parlay::sequence<cluster_graph *> &p, cluster_graph *v) {
  auto u = p.begin();
  ASSERT_MSG(cluster_graph::getLevel(*u) >= cluster_graph::getLevel(v), "u is not belong to higher tree");
  ASSERT_MSG(cluster_graph::getSize(v) + cluster_graph::getSize(*u) <= (1 << cluster_graph::getLevel(*u)),
             "no need to push down");
  while (cluster_graph::isBlocked(*u, v, cluster_graph::getLevel(*u))) {
    ASSERT_MSG(*u != nullptr, "left should be higher than right, always blocked");
    u++;
  }
  ASSERT_MSG(!cluster_graph::isBlocked(*u, v, cluster_graph::getLevel(*u)), "unblocked (*p,v)");
  ASSERT_MSG(cluster_graph::isBlocked(*(u - 1), v, cluster_graph::getLevel(*u - 1)), "blocked (*u,v)");
  return *(u - 1);
}
inline void blocked_arr::insertEdge(size_t u, size_t v, size_t l) {
  leaves[u]->insertToLeaf(v, l);
  leaves[v]->insertToLeaf(u, l);
}
inline void blocked_arr::updateToTop(cluster_graph *src, cluster_graph *dest, size_t incre) {
  auto p = src;
  while (p != nullptr) {
    if (p != src) {
      cluster_graph::increSize(p, incre);
      cluster_graph::sortChild(p);
    }
    if (p != dest)
      cluster_graph::increParentChildSize(p, incre);
    else
      return;
    p = cluster_graph::getParent(p);
  }
}
inline size_t blocked_arr::compressPath(cluster_graph *cu, cluster_graph *cv, cluster_graph *cp) {
  cluster_graph::deleteFromParent(cu, cp);
  if (cluster_graph::getLevel(cu) < cluster_graph::getLevel(cv)) std::swap(cu, cv);
  auto np = cluster_graph::createFromTwo(cu, cv);
  cluster_graph::addChild(cp, np);
  return cluster_graph::getLevel(np);
}