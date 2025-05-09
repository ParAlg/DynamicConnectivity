#include "../helpers/assert.hpp"
#include "absl/container/flat_hash_set.h"
#include "alloc.hpp"
#include "stats.hpp"
#include <bitset>
#include <cassert>
#include <cstdint>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
class localTree;
class rankTree {
public:
  rankTree(uint32_t r, class localTree *_Node, class localTree *_des,
           std::bitset<64> _emap)
      : rank(r), lchild(nullptr), rchild(nullptr), parent(nullptr), Node(_Node),
        descendant(_des), edgemap(_emap) {}
  rankTree(rankTree *T1, rankTree *T2);
  ~rankTree(){};
  using arr = std::vector<rankTree *>;
  static type_allocator<rankTree> *r_alloc;

private:
  friend class localTree;
  rankTree *lchild;
  rankTree *rchild;
  rankTree *parent;
  std::bitset<64> edgemap;
  uint32_t rank;
  // pointer to the local tree node whose father is this rank tree node
  localTree *descendant;
  // This is a pointer to the local tree node containing this rank tree
  localTree *Node;
  const static uint32_t leaf_threshold = 32;
  //
  bool isleaf() { return !lchild && !rchild; }
  void setRank(uint32_t i) { rank = i; }
  uint32_t getRank() { return rank; }
  static arr decompose(rankTree *T, bool clear);
  static arr decompose(arr &rTrees, bool clear);
  static void sortByRank(arr &rTrees);
  static arr buildFromSequence(arr &rTrees, localTree *node);
  static arr remove(rankTree *T, localTree *node);
  static arr remove(arr &rTrees, rankTree *T, localTree *node);
  static arr remove(arr &rTrees, arr &dropped, localTree *node);
  static arr build(arr &rTrees, localTree *node);
  static arr insertRankTree(arr &rTrees, rankTree *T, localTree *node);
  static arr Merge(arr &r1, arr &r2, localTree *node);
  static rankTree *getRoot(rankTree *T);
  static localTree *updateBitMapByTree(rankTree *T, std::bitset<64> nval);
  static void updateBitMap(rankTree *T, std::bitset<64> oval,
                           std::bitset<64> nval);
  static rankTree *fetchLeaf(rankTree *T, uint32_t l);
  static std::bitset<64> getEdgeMap(arr &A);

protected:
  static rankTree *testGetRoot(rankTree *T) { return getRoot(T); }
  static void testRankDistinct(arr &A, std::string msg);
  static void testRankInc(arr &A, std::string msg);
  static arr testRankTreesGen(uint32_t n);
  static arr testBuildFromSequence(arr &A);
  static arr testDecompose(rankTree *T);
  static arr testSortByRank(arr &A);
  static bool testEqualRanks(arr &A, arr &B);
  static arr testRemove(rankTree *T, localTree *node);
  static arr testRemove(arr &rTrees, rankTree *T, localTree *node);
  static arr testMerge(arr &A, arr &B, localTree *node);
};
inline type_allocator<rankTree> *rankTree::r_alloc = nullptr;
inline rankTree::rankTree(rankTree *T1, rankTree *T2) {
  ASSERT_MSG(T1->rank == T2->rank,
             "cannot pair two rankTrees with different rank");
  this->rank = T1->rank + 1;
  // Let the new root tree point to the same local tree node as T1T2
  this->Node = T1->Node;
  this->parent = nullptr;
  this->descendant = nullptr;
  // Apply "or" to two bitmaps. So roots of rankTrees will have the bitmap of
  // its descending leaf. The bitmap of a local tree node will be the or of
  // at most O(logn) rank trees.
  this->edgemap = T1->edgemap | T2->edgemap;
  this->lchild = T1;
  this->rchild = T2;
  T1->parent = this;
  T2->parent = this;
}
inline rankTree::arr rankTree::decompose(rankTree *T, bool clear = false) {
  rankTree::arr rTrees;
  std::queue<rankTree *> Q;
  auto root = T;
  Q.push(root);
  while (!Q.empty()) {
    root = Q.front();
    Q.pop();
    if (root->isleaf()) {
      rTrees.emplace_back(root);
    } else {
      // internal node in rankTree should have two children
      Q.push(root->lchild);
      Q.push(root->rchild);
      if (clear)
        // delete root;
        r_alloc->free(root);
    }
  }
  for (uint32_t i = 0; i < rTrees.size() / 2; i++)
    std::swap(rTrees[i], rTrees[rTrees.size() - i - 1]);
  return std::move(rTrees);
}
inline rankTree::arr rankTree::decompose(arr &rTrees, bool clear = false) {
  if (rTrees.empty())
    return rTrees;
  arr A;
  for (uint32_t i = 0; i < rTrees.size(); i++) {
    auto B = decompose(rTrees[i], clear);
    A.insert(A.end(), B.begin(), B.end());
  }
  sortByRank(A);
  return std::move(A);
}
inline rankTree::arr rankTree::buildFromSequence(arr &rTrees,
                                                 localTree *node = nullptr) {
  arr temp;
  temp.reserve(rTrees.size());
  uint32_t p = 0;
  while (p < rTrees.size()) {
    if (p == rTrees.size() - 1) {
      rTrees[p]->parent = nullptr;
      rTrees[p]->Node = node;
      temp.emplace_back(rTrees[p]);
      break;
    }
    uint32_t counter = 1;
    while ((p + counter) < rTrees.size() &&
           rTrees[p + counter]->rank == rTrees[p]->rank)
      counter++;
    if (counter % 2 == 1) {
      rTrees[p]->parent = nullptr;
      rTrees[p]->Node = node;
      temp.emplace_back(rTrees[p]);
      p++;
      counter--;
    }
    for (uint32_t i = 0; i < counter; i += 2) {
      rTrees[p + i + 1] = r_alloc->create(rTrees[p + i], rTrees[p + i + 1]);
    }
    for (uint32_t i = 0; i < counter / 2; i++) {
      rTrees[(p + counter) - i - 1] = rTrees[(p + counter) - i * 2 - 1];
    }
    p += counter / 2;
  }
  return arr(temp.begin(), temp.begin() + temp.size());
}
inline rankTree::arr rankTree::remove(rankTree *T, localTree *node) {
  rankTree *temp[2 * leaf_threshold];
  uint32_t num_root = 0;
  auto p = T->parent;
  while (p) {
    if (p->lchild == T) {
      p->rchild->Node = node;
      p->rchild->parent = nullptr;
      temp[num_root++] = p->rchild;
    }
    if (p->rchild == T) {
      p->lchild->Node = node;
      p->lchild->parent = nullptr;
      temp[num_root++] = p->lchild;
    }
    r_alloc->free(T);
    T = p;
    p = p->parent;
  }
  r_alloc->free(T);
  return arr(temp, temp + num_root);
}
inline rankTree::arr rankTree::remove(arr &rTrees, rankTree *T,
                                      localTree *node) {
  auto root = getRoot(T);
  for (auto it = rTrees.begin(); it != rTrees.end(); it++) {
    if (*it == root) {
      std::swap(rTrees[it - rTrees.begin()], rTrees[rTrees.size() - 1]);
      rTrees.erase(rTrees.begin() + (rTrees.size() - 1));
      break;
    }
  }
  auto remain = rankTree::remove(T, node);
  return rankTree::Merge(rTrees, remain, node);
}
inline rankTree::arr rankTree::remove(arr &rTrees, arr &dropped,
                                      localTree *node) {
  // for (auto it : dropped)
  //   rTrees = remove(rTrees, it, node);
  // return rTrees;
  absl::flat_hash_set<rankTree *> marked;
  for (auto it : dropped) {
    auto p = it;
    while (p != nullptr && marked.contains(p) == false) {
      marked.emplace(p);
      p = p->parent;
    }
  }
  arr nTrees;
  nTrees.reserve(rTrees.size());
  for (auto it : rTrees)
    if (marked.contains(it) == false)
      nTrees.emplace_back(it);
  for (auto it : dropped) {
    rankTree *r = it;
    while (r != nullptr /*&& marked.contains(r) == true*/) {
      // not neccessary to check r because it and its ancestor all have been
      // marked
      if (r->lchild) {
        r->lchild->parent = nullptr;
        r->lchild->Node = node;
        if (marked.contains(r->lchild) == false)
          nTrees.emplace_back(r->lchild);
      }
      if (r->rchild) {
        r->rchild->parent = nullptr;
        r->rchild->Node = node;
        if (marked.contains(r->rchild) == false)
          nTrees.emplace_back(r->rchild);
      }
      auto p = r->parent;
      if (p) {
        if (p->lchild == r)
          p->lchild = nullptr;
        else
          p->rchild = nullptr;
      }
      r_alloc->free(r);
      r = p;
    }
    if (nTrees.size() > leaf_threshold)
      nTrees = build(nTrees, node);
  }
  return std::move(nTrees);
}
inline rankTree::arr rankTree::insertRankTree(arr &rTrees, rankTree *T,
                                              localTree *node) {
  if (rTrees.size() < leaf_threshold) {
    rTrees.emplace_back(T);
    return std::move(rTrees);
  }
  for (auto it = rTrees.begin(); it != rTrees.end(); it++) {
    if ((*it)->rank >= T->rank) {
      rTrees.insert(it, T);
      return build(rTrees, node);
    }
  }
  rTrees.emplace_back(T);
  return build(rTrees, node);
}
inline void rankTree::sortByRank(arr &rTrees) {
  auto comp = [&](const rankTree *T1, const rankTree *T2) {
    return T1->rank < T2->rank;
  };
  parlay::sort_inplace(rTrees, comp);
}
inline rankTree::arr rankTree::build(arr &rTrees, localTree *node) {
  if (rTrees.size() < leaf_threshold)
    return std::move(rTrees);
  sortByRank(rTrees);
  return buildFromSequence(rTrees, node);
}
// Merge all the nodes from rankTree 2 to rankTree 1.
// Need to modify the pointers to local tree node from r2 to the node from r1
// because we don't update the node pointer of each leaf in r2. Only the root of
// each rankTree stores the correct pointer to its local tree node
inline rankTree::arr rankTree::Merge(arr &r1, arr &r2, localTree *node) {
  if (r1.size() < r2.size())
    return Merge(r2, r1, node);
  r1.reserve(r1.size() + r2.size());
  if (r1[0]->Node != node) {
    for (auto it : r1)
      it->Node = node;
  }
  for (auto it : r2) {
    it->Node = node;
    r1.emplace_back(it);
  }
  return build(r1, node);
}
inline rankTree *rankTree::getRoot(rankTree *T) {
  while (T->parent)
    T = T->parent;
  return T;
}
// if no need to go up, return nullptr
// if reach the root, then return the node this root points to
inline localTree *rankTree::updateBitMapByTree(rankTree *T,
                                               std::bitset<64> nval) {
  if (!T)
    return nullptr;
  T->edgemap = nval;
  while (T->parent) {
    auto oldval = T->parent->edgemap;
    auto lval = (T->parent->lchild != nullptr) ? T->parent->lchild->edgemap : 0;
    auto rval = (T->parent->rchild != nullptr) ? T->parent->rchild->edgemap : 0;
    // no need to go up
    if (oldval == (lval | rval))
      return nullptr;
    T->parent->edgemap = lval | rval;
    T = T->parent;
  }
  assert(T->Node);
  return T->Node;
}
inline rankTree *rankTree::fetchLeaf(rankTree *T, uint32_t l) {
  while (!T->isleaf()) {
    if (T->lchild->edgemap[l] == 1)
      T = T->lchild;
    else if (T->rchild->edgemap[l] == 1)
      T = T->rchild;
    assert(T->edgemap[l] == 1);
  }
  return T;
}
inline std::bitset<64> rankTree::getEdgeMap(arr &A) {
  std::bitset<64> b = 0;
  for (uint32_t i = 0; i < A.size(); i++)
    b = b | A[i]->edgemap;
  return b;
}
inline void rankTree::updateBitMap(rankTree *T, std::bitset<64> oval,
                                   std::bitset<64> nval) {}

inline void rankTree::testRankDistinct(arr &A,
                                       std::string msg = "rank not distince") {
  for (uint32_t i = 0; i < A.size() - 1; i++)
    ASSERT_MSG(A[i]->rank != A[i + 1]->rank, msg);
}
inline void rankTree::testRankInc(arr &A,
                                  std::string msg = "rank not increasing") {
  for (uint32_t i = 0; i < A.size() - 1; i++)
    ASSERT_MSG(A[i]->rank <= A[i + 1]->rank, msg);
}
inline rankTree::arr rankTree::testRankTreesGen(uint32_t n) {
  arr A(n);
  parlay::parallel_for(0, n, [&](uint32_t i) {
    A[i] = new rankTree(parlay::hash64(i) % parlay::log2_up(n), nullptr,
                        nullptr, 0);
  });
  A = testSortByRank(A);
  testRankInc(A, "sort rank fail");
  std::cout << "test rank tree generation" << std::endl;
  // for (uint32_t i = 0; i < n; i++)
  //   std::cout << A[i]->getRank() << std::endl;
  return A;
}
inline rankTree::arr rankTree::testBuildFromSequence(arr &A) {
  std::cout
      << "test building rank trees of distinct rank from a bunch of rank trees"
      << std::endl;
  if (A.size() > 2 * leaf_threshold) {
    std::cout << "this version does not support building from long sequence";
    std::abort();
  }
  auto B = buildFromSequence(A);
  // for (uint32_t i = 0; i < B.size(); i++)
  //   std::cout << B[i]->getRank() << std::endl;
  testRankDistinct(B, "not distinct rank trees");
  return B;
}
inline rankTree::arr rankTree::testDecompose(rankTree *T) {
  return decompose(T);
}
inline rankTree::arr rankTree::testSortByRank(arr &A) {
  sortByRank(A);
  return A;
}
inline bool rankTree::testEqualRanks(arr &A, arr &B) {
  ASSERT_MSG(A.size() == B.size(), "Decompose Size expected to be same");
  // for (uint32_t i = 0; i < A.size(); i++)
  //   std::cout << A[i]->rank << "," << B[i]->rank << std::endl;
  parlay::parallel_for(0, A.size(), [&](uint32_t i) {
    ASSERT_MSG(A[i]->rank == B[i]->rank,
               "rank should be the same after decomposing");
  });
  return true;
}
inline rankTree::arr rankTree::testRemove(rankTree *T,
                                          localTree *node = nullptr) {
  std::cout << "test removing  leaf from a rank tree" << std::endl;
  auto A = remove(T, nullptr);
  testRankDistinct(A, "remove not generate distinct rank trees");
  testRankInc(A, "remove not generate increasing rank trees");
  return A;
}
inline rankTree::arr rankTree::testMerge(arr &A, arr &B,
                                         localTree *node = nullptr) {
  std::cout << "test merging two sequences of rank trees" << std::endl;
  auto C = Merge(A, B, node);
  testRankDistinct(C, "merge does not generate distinct rank trees");
  testRankInc(C, "merge does not generate rank trees in increasing order");
  return C;
}
inline rankTree::arr rankTree::testRemove(arr &rTrees, rankTree *T,
                                          localTree *node = nullptr) {
  return rankTree::remove(rTrees, T, node);
}