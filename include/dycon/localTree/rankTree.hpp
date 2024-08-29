#include "../helpers/assert.hpp"
#include "alloc.h"
#include "stats.hpp"
#include <bitset>
#include <cstdint>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
class localTree;
// todo
//  build
//  insertRankTree
//  merge
//  remove

class rankTree {
public:
  rankTree(uint32_t r, class localTree *_Node, class localTree *_des,
           std::bitset<64> _emap)
      : rank(r), lchild(nullptr), rchild(nullptr), parent(nullptr), Node(_Node),
        descendant(_des), edgemap(_emap) {}
  rankTree(rankTree *T1, rankTree *T2);
  ~rankTree(){};
  using arr = parlay::sequence<rankTree *>;
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

  //
  bool isleaf() { return !lchild && !rchild; }
  void setRank(size_t i) { rank = i; }
  size_t getRank() { return rank; }
  static arr decompose(rankTree *T, bool clear);
  static arr decompose(arr &rTrees, bool clear);
  static arr sortByRank(arr &rTrees);
  static arr buildFromSequence(arr &rTrees, localTree *node);
  static arr remove(rankTree *T, localTree *node);
  static arr remove(arr &rTrees, rankTree *T, localTree *node);
  static arr build(arr &rTrees, localTree *node);
  static arr insertRankTree(arr &rTrees, rankTree *T, localTree *node);
  static arr Merge(arr &r1, arr &r2, localTree *node);
  static rankTree *getRoot(rankTree *T);
  static localTree *updateBitMapByTree(rankTree *T, std::bitset<64> nval);
  static void updateBitMap(rankTree *T, std::bitset<64> oval,
                           std::bitset<64> nval);
  static rankTree *fetchLeaf(rankTree *T, size_t l);
  static std::bitset<64> getEdgeMap(arr &A);

protected:
  static rankTree *testGetRoot(rankTree *T) { return getRoot(T); }
  static void testRankDistinct(arr &A, std::string msg);
  static void testRankInc(arr &A, std::string msg);
  static arr testRankTreesGen(size_t n);
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
    stats::memUsage += sizeof(rankTree);
    Q.pop();
    if (root->isleaf()) {
      rTrees.emplace_back(root);
    } else {
      // internal node in rankTree should have two children
      Q.push(root->lchild);
      Q.push(root->rchild);
      if (clear)
        // delete root;
        r_alloc->deallocate(root);
    }
  }
  for (size_t i = 0; i < rTrees.size() / 2; i++)
    std::swap(rTrees[i], rTrees[rTrees.size() - i - 1]);
  return rTrees;
}
inline rankTree::arr rankTree::decompose(arr &rTrees, bool clear = false) {
  if (rTrees.empty())
    return rTrees;
  arr A;
  for (size_t i = 0; i < rTrees.size(); i++) {
    auto B = decompose(rTrees[i], clear);
    A.append(B);
  }
  return sortByRank(A);
}
inline rankTree::arr rankTree::buildFromSequence(arr &rTrees,
                                                 localTree *node = nullptr) {
  arr Seq;
  size_t p = 0;
  while (p < rTrees.size()) {
    if (p == rTrees.size() - 1) {
      rTrees[p]->parent = nullptr;
      rTrees[p]->Node = node;
      Seq.push_back(rTrees[p]);
      // std::cout << "push " << p << std::endl;
      break;
    }
    size_t counter = 1;
    while ((p + counter) < rTrees.size() &&
           rTrees[p + counter]->rank == rTrees[p]->rank)
      counter++;
    // std::cout << "number of equal rank trees " << counter << std::endl;
    if (counter % 2 == 1) {
      rTrees[p]->parent = nullptr;
      rTrees[p]->Node = node;
      Seq.push_back(rTrees[p]);
      p++;
      counter--;
    }
    for (size_t i = 0; i < counter; i += 2) {
      rTrees[p + i + 1] = r_alloc->construct(rTrees[p + i], rTrees[p + i + 1]);
      // rTrees[p + i + 1] = new rankTree(rTrees[p + i], rTrees[p + i + 1]);
    }
    for (size_t i = 0; i < counter / 2; i++) {
      rTrees[(p + counter) - i - 1] = rTrees[(p + counter) - i * 2 - 1];
      // std::cout << "move from " << (p + counter) - i * 2 - 1 << " to " << (p
      // + counter) - i - 1 << std::endl;
    }
    p += counter / 2;
    // std::cout << " Merge next time happens at" << p << std::endl;
  }
  return Seq;
}
inline rankTree::arr rankTree::remove(rankTree *T, localTree *node) {
  // arr rTrees;
  rankTree *temp[128];
  uint32_t num_root = 0;
  auto p = T->parent;
  while (p) {
    if (p->lchild == T) {
      p->rchild->Node = node;
      p->rchild->parent = nullptr;
      // rTrees.push_back(p->rchild);
      temp[num_root++] = p->rchild;
    }
    if (p->rchild == T) {
      p->lchild->Node = node;
      p->lchild->parent = nullptr;
      // rTrees.push_back(p->lchild);
      temp[num_root++] = p->lchild;
    }
    // delete T;
    r_alloc->deallocate(T);
    T = p;
    p = p->parent;
  }
  // delete T;
  r_alloc->deallocate(T);
  arr rTrees(temp, temp + num_root);
  return rTrees;
}
inline rankTree::arr rankTree::remove(arr &rTrees, rankTree *T,
                                      localTree *node) {
  auto root = getRoot(T);
  for (auto it = rTrees.begin(); it != rTrees.end(); it++) {
    if (*it == root) {
      rTrees.erase(it);
      break;
    }
  }
  auto remain = rankTree::remove(T, node);
  return rankTree::Merge(rTrees, remain, node);
}
inline rankTree::arr rankTree::insertRankTree(arr &rTrees, rankTree *T,
                                              localTree *node) {
  for (auto it = rTrees.begin(); it != rTrees.end(); it++) {
    if ((*it)->rank >= T->rank) {
      rTrees.insert(it, T);
      return build(rTrees, node);
    }
  }
  rTrees.push_back(T);
  return build(rTrees, node);
}
inline rankTree::arr rankTree::sortByRank(arr &rTrees) {
  auto comp = [&](const rankTree *T1, const rankTree *T2) {
    return T1->rank < T2->rank;
  };
  parlay::sort_inplace(rTrees, comp);
  return rTrees;
}
inline rankTree::arr rankTree::build(arr &rTrees, localTree *node) {
  rankTree *temp[128];
  uint32_t num_root = 0;
  if (!rTrees.size())
    return rTrees;
  for (size_t i = 0; i < rTrees.size() - 1; i++) {
    if (rTrees[i]->rank == rTrees[i + 1]->rank)
      // rTrees[i + 1] = new rankTree(rTrees[i], rTrees[i + 1]);
      rTrees[i + 1] = r_alloc->construct(rTrees[i], rTrees[i + 1]);
    else {
      if (rTrees[i]->rank > rTrees[i + 1]->rank)
        std::swap(rTrees[i], rTrees[i + 1]);
      rTrees[i]->Node = node;
      rTrees[i]->parent = nullptr;
      // Seq.push_back(rTrees[i]);
      temp[num_root++] = rTrees[i];
    }
  }
  rTrees[rTrees.size() - 1]->Node = node;
  rTrees[rTrees.size() - 1]->parent = nullptr;
  // Seq.push_back(rTrees[rTrees.size() - 1]);
  temp[num_root++] = rTrees[rTrees.size() - 1];
  arr Seq(temp, temp + num_root);
  return Seq;
}
// Merge all the nodes from rankTree 2 to rankTree 1.
// Need to modify the pointers to local tree node from r2 to the node from r1
// because we don't update the node pointer of each leaf in r2. Only the root of
// each rankTree stores the correct pointer to its local tree node
inline rankTree::arr rankTree::Merge(arr &r1, arr &r2, localTree *node) {
  // arr Seq;
  rankTree *temp[128];
  uint32_t num_root = 0;
  size_t p = 0, q = 0;
  while (p < r1.size() || q < r2.size()) {
    if (p == r1.size()) {
      while (q < r2.size()) {
        r2[q]->Node = node;
        r2[q]->parent = nullptr;
        // Seq.push_back(r2[q]);
        temp[num_root++] = r2[q];
        q++;
      }
      break;
    }
    if (q == r2.size()) {
      while (p < r1.size()) {
        r1[p]->Node = node;
        r1[p]->parent = nullptr;
        // Seq.push_back(r1[p]);
        temp[num_root++] = r1[p];
        p++;
      }
      break;
    }
    if (r1[p]->rank > r2[q]->rank) {
      r2[q]->Node = node;
      r2[q]->parent = nullptr;
      // Seq.push_back(r2[q]);
      temp[num_root++] = r2[q];
      q++;
    } else {
      r1[p]->Node = node;
      r1[p]->parent = nullptr;
      // Seq.push_back(r1[p]);
      temp[num_root++] = r1[p];
      p++;
    }
  }
  arr Seq(temp, temp + num_root);
  return build(Seq, node);
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
    // std::cout << "old " << oldval << " new " << nval << " lval " << lval << "
    // rval" << rval << std::endl;
    T->parent->edgemap = lval | rval;
    T = T->parent;
  }
  assert(T->Node);
  return T->Node;
}
inline rankTree *rankTree::fetchLeaf(rankTree *T, size_t l) {
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
  for (size_t i = 0; i < A.size(); i++)
    b = b | A[i]->edgemap;
  return b;
}
inline void rankTree::updateBitMap(rankTree *T, std::bitset<64> oval,
                                   std::bitset<64> nval) {}

inline void rankTree::testRankDistinct(arr &A,
                                       std::string msg = "rank not distince") {
  for (size_t i = 0; i < A.size() - 1; i++)
    ASSERT_MSG(A[i]->rank != A[i + 1]->rank, msg);
}
inline void rankTree::testRankInc(arr &A,
                                  std::string msg = "rank not increasing") {
  for (size_t i = 0; i < A.size() - 1; i++)
    ASSERT_MSG(A[i]->rank <= A[i + 1]->rank, msg);
}
inline rankTree::arr rankTree::testRankTreesGen(size_t n) {
  arr A(n);
  parlay::parallel_for(0, n, [&](size_t i) {
    A[i] = new rankTree(parlay::hash64(i) % parlay::log2_up(n), nullptr,
                        nullptr, 0);
  });
  A = testSortByRank(A);
  testRankInc(A, "sort rank fail");
  std::cout << "test rank tree generation" << std::endl;
  // for (size_t i = 0; i < n; i++)
  //   std::cout << A[i]->getRank() << std::endl;
  return A;
}
inline rankTree::arr rankTree::testBuildFromSequence(arr &A) {
  std::cout
      << "test building rank trees of distinct rank from a bunch of rank trees"
      << std::endl;
  auto B = buildFromSequence(A);
  // for (size_t i = 0; i < B.size(); i++)
  //   std::cout << B[i]->getRank() << std::endl;
  testRankDistinct(B, "not distinct rank trees");
  return B;
}
inline rankTree::arr rankTree::testDecompose(rankTree *T) {
  return decompose(T);
}
inline rankTree::arr rankTree::testSortByRank(arr &A) {
  sortByRank(A);

  // for (size_t i = 0; i < A.size(); i++)
  //   std::cout << A[i]->rank << ",";
  // std::cout << std::endl;

  return A;
}
inline bool rankTree::testEqualRanks(arr &A, arr &B) {
  ASSERT_MSG(A.size() == B.size(), "Decompose Size expected to be same");
  // for (size_t i = 0; i < A.size(); i++)
  //   std::cout << A[i]->rank << "," << B[i]->rank << std::endl;
  parlay::parallel_for(0, A.size(), [&](size_t i) {
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