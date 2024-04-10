#include "cluster_graph.hpp"
#include <parlay/alloc.h>
class blocked_arr {
 public:
  parlay::sequence<cluster_graph *> leaves;
  blocked_arr(size_t n) : leaves(parlay::sequence<cluster_graph *>(n, nullptr)){};
  void insert(size_t u, size_t v);

  bool is_connected(size_t u, size_t v);
  void statistic(bool clear, bool stat, bool verbose);
  void run_stat() { statistic(false, true, false); }
  static void printCG(parlay::sequence<cluster_graph *> &path);
  ~blocked_arr() { statistic(true, true, false); }

 private:
  struct pathblock {
    char x[960];
  };
  void insert_bk(size_t u, size_t v);
  using pathAllocator = parlay::type_allocator<pathblock>;
  static cluster_graph **alloc() { return (cluster_graph **)pathAllocator::alloc(); }
  static void dealloc(cluster_graph **p) { pathAllocator::free((pathblock *)p); }
  static void updateToTop(cluster_graph *src, cluster_graph *dest, size_t inc);
  static size_t pushDown(cluster_graph **path, size_t Len, cluster_graph *v);
  static cluster_graph *pushDown_bk(parlay::sequence<cluster_graph *> &p, cluster_graph *v);
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
        if (cluster_graph::getSize(lca) > (1 << cluster_graph::getLevel(lca))) {
          cluster_graph::cleanTopDown(lca, false, nullptr, false, true);
        }
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
inline void blocked_arr::insert_bk(size_t u, size_t v) {
  auto g = [&](size_t x) {
    if (leaves[x] == nullptr) {
      leaves[x] = new cluster_graph(0, x, 1);  // level 0 with size 1
    }
    parlay::sequence<cluster_graph *> path;
    // cluster_graph::getRootPath(leaves[x], path);  // get path from leaf to root
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
  // std::cout << "u = " << u << " v = " << v << std::endl;
  if (*iu != *iv) {  // not connected
    if (cluster_graph::isBlocked(*iu, *iv, cluster_graph::getLevel(*iu))) {
      // case 1 2
      // insert edge (u,v) at level parent->level
      // std::cout << "unconnected 1\n";
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
      auto cu = pushDown_bk(pu, cv);
      auto cp = cluster_graph::getParent(cu);
      // cu cv block at lcu, cp cv unblock at lcp cu cv may block at lcv
      auto incre = cluster_graph::getSize(cv);
      size_t level = 0;
      if (cluster_graph::getLevel(cp) - cluster_graph::getLevel(cu) == 1) {
        // std::cout << "unconnected 2\n";
        ASSERT_MSG(cluster_graph::getLevel(cp) > cluster_graph::getLevel(cv), "connect cv as cp's child wrong");
        cluster_graph::addChild(cp, cv);
        level = cluster_graph::getLevel(cp);

      } else {
        // std::cout << "unconnected 3\n";
        cluster_graph::cutChild(cu, cp);
        if (cluster_graph::getLevel(cu) < cluster_graph::getLevel(cv)) std::swap(cu, cv);
        level = compressPath(cu, cv, cp);
      }
      // update to root *(pu.rbegin())
      updateToTop(cp, *iu, incre);
      insertEdge(u, v, level);
    }
  } else {  // u,v  connected
    // find lca(u,v) and put it into lca
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

    if (cluster_graph::isBlocked(*iu, *iv, cluster_graph::getLevel(*iu))) {
      // std::cout << "connected 3\n";
      // blocked edge goes up
      if (cluster_graph::getLevel(lca) - cluster_graph::getLevel(*iu) == 1) {
        // no compression
        // std::cout << "connected no compress\n";
        std::cout << "connected 1\n";
        insertEdge(u, v, cluster_graph::getLevel(lca));
      } else {
        // compression
        // delete *iu, create new tree from iu iv and plug into parent's children

        // avoid double decre
        // if (cluster_graph::getParent(*iv) != lca) cluster_graph::decreSize(lca, cluster_graph::getSize(*iv));
        cluster_graph::cutChild(*iv, lca);
        auto cv = *iv;
        // std::cout << " &u = " << *pu.rbegin() << " with level " << cluster_graph::getLevel(*pu.rbegin())
        //           << " with size " << cluster_graph::getSize(*pu.rbegin()) << " &v = " << *iv << " with level "
        //           << cluster_graph::getLevel(*iv) << " with size " << cluster_graph::getSize(*iv)
        //           << " &parent = " << lca << " with level " << cluster_graph::getLevel(lca) << " with size "
        //           << cluster_graph::getSize(lca) << std::endl;
        size_t level;
        auto cu = pushDown_bk(pu, cv);
        // cp might be lca or not
        // since l(lca) - l(cu) > 1, need to create a new parent between cu and lca
        auto cp = cluster_graph::getParent(cu);
        ASSERT_MSG(cluster_graph::getLevel(cp) > cluster_graph::getLevel(cu), "!lcp>lcu");
        auto incre = cluster_graph::getSize(cv);
        if (cluster_graph::getLevel(cp) - cluster_graph::getLevel(cu) == 1) {
          std::cout << "connected 2\n";
          cluster_graph::addChild(cp, cv);
          level = cluster_graph::getLevel(cp);
        } else {
          std::cout << "connected 3\n";
          level = compressPath(cu, cv, cp);
        }
        updateToTop(cp, lca, incre);
        insertEdge(u, v, level);
        // auto l = cluster_graph::compressPath(*iu, *iv, lca);
        // leaves[u]->insertToLeaf(v, l);
        // leaves[v]->insertToLeaf(u, l);
        // auto src = cluster_graph::getParent(cluster_graph::getParent(*iv));
        // if (src) cluster_graph::updateToTop(src, parent, cluster_graph::getSize(*iv));
        // update to lca parent
      }
    } else {
      std::cout << "connected4\n";
      cluster_graph::cutChild(*iv, lca);
      size_t level;
      auto cv = *iv;
      auto cu = pushDown_bk(pu, cv);
      auto cp = cluster_graph::getParent(cu);
      auto incre = cluster_graph::getSize(cv);
      if (cluster_graph::getLevel(cp) - cluster_graph::getLevel(cu) == 1) {
        cluster_graph::addChild(cp, cv);
        level = cluster_graph::getLevel(cp);
      } else {
        level = compressPath(cu, cv, cp);
      }
      updateToTop(cp, lca, incre);
      insertEdge(u, v, level);
    }
    // else {
    //   // unblocked edge goes down
    //   //  delete *iv, link to the path of *iu
    //   // cut from leaf to *iu;
    //   //  p = lca(u,v)
    //   //  std::cout << "connected compress\n";
    //   // u v not block
    //   cluster_graph::cutChild(*iv, lca);
    //   auto l = cluster_graph::pushDownToBeChild(pu, *iv, parent);
    //   auto src = cluster_graph::getParent(cluster_graph::getParent(*iv));
    //   if (src) cluster_graph::updateToTop(src, parent, cluster_graph::getSize(*iv));
    //   leaves[u]->insertToLeaf(v, l);
    //   leaves[v]->insertToLeaf(u, l);
    //   // update to lca parent
    // }
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
inline cluster_graph *blocked_arr::pushDown_bk(parlay::sequence<cluster_graph *> &p, cluster_graph *v) {
  ASSERT_MSG(cluster_graph::getLevel(*p.rbegin()) >= cluster_graph::getLevel(v), "u is not belong to higher tree");

  auto u = p.begin();
  while (cluster_graph::isBlocked(*u, v, cluster_graph::getLevel(*u))) {
    if (*(u + 1) == nullptr) {
      cluster_graph::cleanTopDown(*u, false, nullptr, false, true);
      cluster_graph::cleanTopDown(v, false, nullptr, false, true);
    }
    u++;
    ASSERT_MSG(*u != nullptr, "left should be higher than right, always blocked");
  }
  ASSERT_MSG(!cluster_graph::isBlocked(*u, v, cluster_graph::getLevel(*u)), "unblocked (*p,v)");
  ASSERT_MSG(cluster_graph::isBlocked(*(u - 1), v, cluster_graph::getLevel(*(u - 1))), "blocked (*u,v)");
  return *(u - 1);
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
        cluster_graph::cleanTopDown(cluster_graph::getRoot(p), false, nullptr, false, true);
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
inline size_t blocked_arr::compressPath(cluster_graph *cu, cluster_graph *cv, cluster_graph *cp) {
  if (cluster_graph::getLevel(cp) - cluster_graph::getLevel(cv) <= 1) {
    cluster_graph::cleanTopDown(cluster_graph::getRoot(cp), false, nullptr, false, true);
    cluster_graph::cleanTopDown(cluster_graph::getRoot(cv), false, nullptr, false, true);
    cluster_graph::cleanTopDown(cu, false, nullptr, false, true);
  }
  cluster_graph::deleteFromParent(cu, cp);

  auto np = cluster_graph::createFromTwo(cu, cv);
  cluster_graph::addChild(cp, np);

  // if (cluster_graph::getLevel(cp) <= cluster_graph::getLevel(np)) {
  //   cluster_graph::cleanTopDown(cp, false, nullptr, false, true);
  //   cluster_graph::cleanTopDown(cu, false, nullptr, false, true);
  //   cluster_graph::cleanTopDown(cluster_graph::getRoot(cp), false, nullptr, false, true);
  // }

  ASSERT_MSG(cluster_graph::getLevel(cp) > cluster_graph::getLevel(np), "lcp<=lnp");
  return cluster_graph::getLevel(np);
}