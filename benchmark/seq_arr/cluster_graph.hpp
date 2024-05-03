#include "assert.hpp"
#include "leaf.hpp"
#include "stats.hpp"
#include <bitset>
#include <parlay/primitives.h>
class cluster_graph {
 public:
  class child {
   public:
    cluster_graph* host;
    cluster_graph* descent;
    std::bitset<64> edgemap;
    size_t size;
    child(cluster_graph* h = nullptr, cluster_graph* d = nullptr, std::bitset<64> b = 0, size_t s = 0) :
        host(h), descent(d), edgemap(b), size(s){};
  };
  using children = parlay::sequence<child*>;
  friend class stats;
  // class stat {
  //  public:
  //   class info_per_node {
  //    public:
  //     size_t size;
  //     size_t NumChild;
  //     info_per_node(size_t s, size_t n) : size(s), NumChild(n) {}
  //   };
  //   parlay::sequence<std::pair<size_t, info_per_node>> info;

  // void printStat(size_t i) {
  // size_t threshold = 5;
  // parlay::sort_inplace(info, [&](std::pair<size_t, info_per_node> a, std::pair<size_t, info_per_node> b) {
  //   return a.first > b.first;
  // });
  // if (info[0].first < threshold) return;
  // std::ofstream fout;
  // fout.open("orkut/" + std::to_string(i) + ".txt");
  // for (auto& it : info) {
  //   if (it.first < threshold) break;
  //   fout << it.first << " " << it.second.size << " " << it.second.NumChild << std::endl;
  // }

  // fout.close();
  // }
  // }
  // ;

  cluster_graph(size_t l = 0, size_t sz = 0, std::bitset<64> b = 0, size_t nc = 0) :
      level(l), size(sz), edgemap(b), NumChild(nc), parent(nullptr), lf(nullptr), nodes() {}
  cluster_graph(size_t l = 0, size_t id = 0, size_t sz = 1) :
      edgemap(0), size(sz), level(l), parent(nullptr), lf(nullptr), NumChild(0), nodes() {
    if (l == 0) lf = new leaf(id, this);
  }
  ~cluster_graph() { this->nodes.clear(); }

  static children mergeChildren(children& l, children& r);
  static std::bitset<64> updateBitMap(children& nodes);
  void testMakeArray(size_t n);
  void printNodeSize(size_t n);
  void testMakeBitMap(size_t n);
  void printBitMap(size_t n);
  void insertToLeaf(size_t e, size_t l);
  static size_t getLevel(cluster_graph* v) { return v->level; }
  static void insertChild(child* ch, cluster_graph* cg);
  static void deleteChild(child* ch, cluster_graph* cg);
  static void deleteFromParent(cluster_graph* u, cluster_graph* p) { deleteChild(u->parent, p); }
  static void cutChild(cluster_graph* ch, cluster_graph* p);
  static size_t getRootPathLen(cluster_graph* v, cluster_graph** path);
  static cluster_graph* getRoot(cluster_graph* v);
  static bool notViolateSize(cluster_graph* u, cluster_graph* v);
  static bool atSameLevel(cluster_graph* u, cluster_graph* v) { return u->level == v->level; }
  static bool uLevelSmall(cluster_graph* u, cluster_graph* v) { return u->level < v->level; }
  static cluster_graph* createFromTwo(cluster_graph* u, cluster_graph* v);
  static size_t pushDownToBeChild(parlay::sequence<cluster_graph*>& pu, cluster_graph* v, cluster_graph* dest);
  static size_t compressPath(cluster_graph* u, cluster_graph* v, cluster_graph* p);
  static bool isBlocked(cluster_graph* u, cluster_graph* v, size_t l) { return u->size + v->size > (1 << l); }
  static void addChild(cluster_graph* p, cluster_graph* v);
  static size_t getSize(cluster_graph* v) { return v->size; }
  static void updateToTop(cluster_graph* src, cluster_graph* dest, size_t incre);
  static cluster_graph* pushDown(parlay::sequence<cluster_graph*>& pu, cluster_graph* v);
  static void cleanTopDown(cluster_graph* v, bool clear, parlay::sequence<stats>& st, bool flag, bool verbose);
  static cluster_graph* getAncestor(cluster_graph* v, size_t l);
  static size_t getEdgeLevel(cluster_graph* v, size_t u) {
    if (v->lf) return v->lf->getLevel(u);
    return UINT64_MAX;
  }
  static void sortChild(cluster_graph* v) {
    parlay::sort_inplace(v->nodes, [&](child* l, child* r) { return l->size < r->size; });
  }
  static cluster_graph* getParent(cluster_graph* v) {
    if (!v || v->parent == nullptr) return nullptr;
    return v->parent->host;
  }
  static void increSize(cluster_graph* v, size_t size) { v->size += size; }
  static void decreSize(cluster_graph* v, size_t size) { v->size -= size; }
  static void increParentChildSize(cluster_graph* v, size_t size) {
    if (v->parent) v->parent->size += size;
  }
  static void decreParentChildSize(cluster_graph* v, size_t n) {
    if (v->parent) v->parent->size -= n;
  }
  static void cleanMM(cluster_graph* v) {
    while (v) {
      std::cout << v << " is freed with " << v->nodes.size() << " children\n";
      if (!v->nodes.empty()) {
        for (size_t i = 0; i < v->nodes.size(); i++) {
          std::cout << "i = " << i << std::endl;
          v->nodes[i]->descent->parent = nullptr;
          delete v->nodes[i];
        }
        v->nodes.clear();
      }
      auto p = v;
      if (v->parent)
        v = v->parent->host;
      else
        break;
      delete p;
    }
  }

 private:
  children nodes;
  size_t NumChild;
  std::bitset<64> edgemap;
  size_t size;
  size_t level;
  child* parent;
  leaf* lf;
};
inline cluster_graph::children cluster_graph::mergeChildren(children& l, children& r) {
  return parlay::merge(l, r, [&](child* l, child* r) { return l->size < r->size; });
}
// update bitmap within a node
inline std::bitset<64> cluster_graph::updateBitMap(children& nodes) {
  return parlay::reduce(parlay::tabulate(nodes.size(), [&](size_t i) { return nodes[i]->edgemap; }),
                        parlay::bit_or<std::bitset<64>>());
}
inline void cluster_graph::testMakeArray(size_t n) {
  nodes = parlay::sort(parlay::tabulate(n,
                                        [&](size_t i) {
                                          child* t = new child(nullptr, nullptr, 0, parlay::hash64(i) % n);
                                          return t;
                                        }),
                       [&](child* l, child* r) { return l->size < r->size; });
  parlay::parallel_for(0, nodes.size() - 1,
                       [&](size_t i) { ASSERT_MSG(nodes[i]->size <= nodes[i + 1]->size, "not sorted"); });
}

inline void cluster_graph::printNodeSize(size_t n) {
  auto g = parlay::tabulate(n, [&](size_t i) { return nodes[i]->size; });
  std::copy(g.begin(), g.end(), std::ostream_iterator<size_t>(std::cout, ","));
  std::cout << "\n\n";
}
inline void cluster_graph::printBitMap(size_t n) {
  auto g = parlay::tabulate(n, [&](size_t i) { return nodes[i]->edgemap; });
  std::copy(g.begin(), g.end(), std::ostream_iterator<std::bitset<64>>(std::cout, ","));
  std::cout << "\n";
}
inline void cluster_graph::testMakeBitMap(size_t n) {
  nodes = parlay::tabulate(n, [&](size_t i) {
    child* t = new child(nullptr, nullptr, parlay::hash64_2(i) % n, 0);
    return t;
  });
  std::bitset<64> flag(0);
  for (size_t i = 0; i < n; i++)
    flag = flag | nodes[i]->edgemap;
  printBitMap(n);
  std::cout << flag << " , " << updateBitMap(nodes) << "\n\n";
  ASSERT_MSG(flag == updateBitMap(nodes), "update bitmap fails");
}
inline void cluster_graph::insertToLeaf(size_t e, size_t l) {
  ASSERT_MSG(this->lf != nullptr, "not insert to leaf");
  this->lf->insert(e, l);
  this->edgemap = this->lf->getEdgeMap();
  size_t i = 0;
  auto p = this;
  while (p) {
    if (!p->nodes.empty()) p->edgemap = updateBitMap(p->nodes);
    if (p->parent) {
      p->parent->edgemap = p->edgemap;
      p = p->parent->host;
    } else
      break;
  }
}
inline size_t cluster_graph::getRootPathLen(cluster_graph* v, cluster_graph** path) {
  size_t length = 0;
  while (v) {
    path[length++] = v;
    v = v->getParent(v);
  }
  return length;
}
inline cluster_graph* cluster_graph::getRoot(cluster_graph* v) {
  while (v->parent) {
    v = v->parent->host;
  }
  return v;
}
inline cluster_graph* cluster_graph::getAncestor(cluster_graph* v, size_t l) {
  while (v->parent) {
    auto p = v->parent->host;
    if (p->level == l) return v;
    v = p;
  }
  return v;  // shouldn't reach here
}
inline bool cluster_graph::notViolateSize(cluster_graph* u, cluster_graph* v) {
  size_t l = std::max(u->level, v->level);
  if (u->size + v->size < (2 << l)) return true;
  return false;
}
inline void cluster_graph::insertChild(cluster_graph::child* ch, cluster_graph* cg) {
  cg->size = cg->size + ch->size;

  ASSERT_MSG(cg->size <= (1 << cg->level), "violate size during insertChild");
  cg->NumChild++;
  // for (auto it = cg->nodes.begin(); it < cg->nodes.end(); it++) {
  //   if ((*it)->size >= ch->size) {
  //     cg->nodes.insert(it, ch);
  //     cg->edgemap = updateBitMap(cg->nodes);
  //     return;
  //   }
  // }
  cg->nodes.emplace_back(ch);
  // cg->edgemap = updateBitMap(cg->nodes);
}
inline void cluster_graph::deleteChild(cluster_graph::child* ch, cluster_graph* cg) {
  cg->size = cg->size - ch->size;
  cg->NumChild--;
  // if (cg->parent) cg->parent->size -= ch->size;
  for (auto it = cg->nodes.begin(); it < cg->nodes.end(); it++) {
    if (*it == ch) {
      delete ch;
      cg->nodes.erase(it);
      // cg->edgemap = updateBitMap(cg->nodes);

      return;
    }
  }
  std::cerr << "delete failed\n";
}
// cut ch from children of p
inline void cluster_graph::cutChild(cluster_graph* ch, cluster_graph* p) {
  ASSERT_MSG(ch->parent->host == p, "cutChild wrong place");
  deleteChild(ch->parent, p);
  ch->parent = nullptr;
}
inline cluster_graph* cluster_graph::createFromTwo(cluster_graph* u, cluster_graph* v) {
  ASSERT_MSG(u->level >= v->level, "cannot create when level is different!");
  // set children of parent to iu,iv, point iu iv to parent

  auto parent = new cluster_graph(std::max(u->level, v->level) + 1, u->size + v->size, u->edgemap | v->edgemap, 2);
  if ((u->size + v->size) > (1 << (u->level + 1))) {
    std::cout << "u->size = " << u->size << " u->level = " << u->level << " v->size = " << v->size
              << " v->level = " << v->level << std::endl;
    // cleanTopDown(u, false, nullptr, false, true);
  }

  ASSERT_MSG(u->size + v->size <= (1 << (u->level + 1)), "violate size create from two");
  // std::cout << u << v << parent << std::endl;
  auto c1 = new cluster_graph::child(parent, u, u->edgemap, u->size);
  auto c2 = new cluster_graph::child(parent, v, v->edgemap, v->size);
  u->parent = c1;
  v->parent = c2;

  if (u->size > v->size) std::swap(c1, c2);
  parent->nodes.push_back(c1);
  parent->nodes.push_back(c2);
  return parent;
}
inline void cluster_graph::cleanTopDown(cluster_graph* v, bool clear, parlay::sequence<stats>& st, bool runStat,
                                        bool verbose) {
  // ASSERT_MSG(v->nodes.size() == v->NumChild, "node children container wrong");
  // ASSERT_MSG(v->size <= (1 << v->level), "node size violate");
  if (v->lf && clear) delete v->lf;
  stats::memUsage += sizeof(cluster_graph) + v->NumChild * sizeof(child) + v->NumChild * sizeof(child*);
  if (verbose) {
    std::cout << "freeing " << v << " at level " << v->level << " with size " << v->size << std::endl;
    for (size_t i = 0; i < v->NumChild; i++)
      std::cout << v->nodes[i]->descent << ",";
    std::cout << "\n\n";
  }
  if (runStat) {
    stats info(v->level, v->size, v->NumChild);
    st.push_back(std::move(info));
  }
  size_t count = 0;
  for (size_t i = 0; i < v->nodes.size(); i++) {
    count += v->nodes[i]->size;
    cleanTopDown(v->nodes[i]->descent, clear, st, runStat, verbose);
    if (clear) delete v->nodes[i];
  }
  if (v->level == 0) count = 1;
  if (count != v->size) std::cout << count << "," << v->size << std::endl;
  ASSERT_MSG(count == v->size, "nodes size != sum of children size");
  if (clear) {
    v->nodes.clear();
    delete v;
  }
}
// Add v as a child of p
inline void cluster_graph::addChild(cluster_graph* p, cluster_graph* v) {
  auto c = new child(p, v, v->edgemap, v->size);
  v->parent = c;
  insertChild(c, p);
}
