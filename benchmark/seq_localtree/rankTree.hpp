#include "assert.hpp"
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <bitset>
#include <queue>
#include <deque>
typedef enum { VERTEX, NODE } MODE;
class rankTree {
 public:
  // This is the rank tree data structure descriped in Section3.4
  // If a rank tree node is leaf or the rank tree only contains
  // a root node, then the content of this node should be the cluster
  // in next level or the leaf in the whole cluster tree which is the
  // vertex of the graph
  rankTree(size_t r, void* _Node, void* _leaf, std::bitset<64> _emap) :
      rank(r), lchild(nullptr), rchild(nullptr), parent(nullptr), Node(_Node), leaf(_leaf), edgemap(_emap) {}
  // Used in the constructor of cluster forest
  rankTree(size_t r = 0) : rank(r), lchild(nullptr), rchild(nullptr), parent(nullptr) {}
  rankTree(rankTree* T1, rankTree* T2);
  void setLeaf(void* p) { leaf = p; }
  void setNode(void* p) { Node = p; }
  bool isleaf() { return !lchild && !rchild; }
  bool checkLevelEdge(size_t l) { return edgemap[l]; }
  void* getLeaf() { return leaf; }
  void* getNode() { return Node; }
  size_t getRank() const { return rank; }
  static rankTree* getRoot(rankTree* T);
  static std::vector<rankTree*> remove(rankTree* T, void* node);
  static std::vector<rankTree*> decompose(rankTree* T);
  static std::vector<rankTree*> buildFromSequence(std::vector<rankTree*>& rTrees);
  static std::vector<rankTree*> build(std::vector<rankTree*>& rTrees, void* node);
  static std::vector<rankTree*> Merge(std::vector<rankTree*>& r1, std::vector<rankTree*>& r2, void* node);
  // static std::bitset<64> updateBitMap(std::vector<rankTree*>& rTrees);
  static rankTree* updateBitMap(rankTree* r, bool nval, size_t l);
  static rankTree* getLeftMostLeaf(rankTree* p);
  static rankTree* getNextLeaf(rankTree* p);

//  private:
  rankTree* lchild;
  rankTree* rchild;
  rankTree* parent;
  std::bitset<64> edgemap;
  // The leaf may be a node or vertex
  void* leaf;
  // This is a pointer to the node containing this rank tree
  void* Node;
  size_t rank;
};
inline rankTree* rankTree::getRoot(rankTree* T) {
  while (T->parent) {
    T = T->parent;
  }
  return T;
}
inline rankTree::rankTree(rankTree* T1, rankTree* T2) {
  ASSERT_MSG(T1->rank == T2->rank, "cannot pair two rankTrees with different rank");
  this->rank = T1->rank + 1;
  // Let the new root tree point to the same local tree node as T1T2
  this->Node = T1->Node;
  this->parent = nullptr;
  // Apply "or" to two bitmaps. So roots of rankTrees will have the bitmap of
  // its descending leaf. The bitmap of a local tree node will be the or of
  // at most O(logn) rank trees.
  this->edgemap = T1->edgemap | T2->edgemap;
  this->lchild = T1;
  this->rchild = T2;
  T1->parent = this;
  T2->parent = this;
};
inline std::vector<rankTree*> rankTree::decompose(rankTree* T) {
  // We don't know how many children a cluster node can have, so this will break the logn limit.
  std::queue<rankTree*> BFS;
  std::vector<rankTree*> rTrees;
  rankTree* root = T;

  BFS.push(root);
  while (!BFS.empty()) {
    // If we reach leaf and the leaf is different from b;
    root = BFS.front();
    // std::cout << "root: " << root << std::endl;
    BFS.pop();
    if (!root->lchild && !root->rchild) {
      rTrees.push_back(root);
      continue;
    }
    if (root->lchild) BFS.push(root->lchild);
    if (root->rchild) BFS.push(root->rchild);
    // delete root;
  }
  // the rank trees are in decreasing order, we need to reverse it.
  for (size_t i = 0; i < rTrees.size() / 2; i++)
    std::swap(rTrees[i], rTrees[rTrees.size() - i - 1]);
  // std::cout << "rTrees size " << rTrees.size() << std::endl;
  return rTrees;
}
// In section 3.6 We delete the path from b to the root of Tb,
// thereby partitioning this rank tree  to O(logn) smaller sorted rank trees.
// No need to modify the pointer to local tree node because it only involves one node.

inline std::vector<rankTree*> rankTree::remove(rankTree* T, void* node) {
  std::vector<rankTree*> rTrees;
  rTrees.reserve(32);
  // std::cout << "address of T " << T << std::endl;
  // std::cout << "address of T->parent " << T->parent << std::endl;
  auto p = T->parent;
  while (p) {
    if (p->lchild == T) {
      p->rchild->Node = node;
      p->rchild->parent = nullptr;
      rTrees.push_back(p->rchild);
    }
    if (p->rchild == T) {
      p->lchild->Node = node;
      p->lchild->parent = nullptr;
      rTrees.push_back(p->lchild);
    }
    delete T;
    T = p;
    p = p->parent;
  }
  delete T;
  // std::cout << "rankTree alive " << rTrees.size() << std::endl;
  return build(rTrees, node);
}
inline std::vector<rankTree*> rankTree::buildFromSequence(std::vector<rankTree*>& rTrees) {
  std::vector<rankTree*> Seq;
  Seq.reserve(rTrees.size());
  size_t p = 0;
  while (p < rTrees.size()) {
    if (p == rTrees.size() - 1) {
      rTrees[p]->parent = nullptr;
      Seq.push_back(rTrees[p]);
      // std::cout << "push " << p << std::endl;
      break;
    }
    size_t counter = 1;
    while ((p + counter) < rTrees.size() && rTrees[p + counter]->rank == rTrees[p]->rank)
      counter++;
    // std::cout << "number of equal rank trees " << counter << std::endl;
    if (counter % 2 == 1) {
      rTrees[p]->parent = nullptr;
      Seq.push_back(rTrees[p]);
      p++;
      counter--;
    }
    for (size_t i = 0; i < counter; i += 2) {
      rTrees[p + i + 1] = new rankTree(rTrees[p + i], rTrees[p + i + 1]);
    }
    for (size_t i = 0; i < counter / 2; i++) {
      rTrees[(p + counter) - i - 1] = rTrees[(p + counter) - i * 2 - 1];
      // std::cout << "move from " << (p + counter) - i * 2 - 1 << " to " << (p + counter) - i - 1 << std::endl;
    }
    p += counter / 2;
    // std::cout << " Merge next time happens at" << p << std::endl;
  }
  return Seq;
}
inline std::vector<rankTree*> rankTree::build(std::vector<rankTree*>& rTrees, void* node) {
  std::vector<rankTree*> Seq;
  if (!rTrees.size()) return Seq;
  Seq.reserve(rTrees.size());
  for (size_t i = 0; i < rTrees.size() - 1; i++) {
    if (rTrees[i]->rank == rTrees[i + 1]->rank)
      rTrees[i + 1] = new rankTree(rTrees[i], rTrees[i + 1]);
    else {
      if (rTrees[i]->rank > rTrees[i + 1]->rank) std::swap(rTrees[i], rTrees[i + 1]);
      rTrees[i]->Node = node;
      rTrees[i]->parent = nullptr;
      Seq.push_back(rTrees[i]);
    }
  }
  rTrees[rTrees.size() - 1]->Node = node;
  rTrees[rTrees.size() - 1]->parent = nullptr;
  Seq.push_back(rTrees[rTrees.size() - 1]);
  return Seq;
}
// Merge all the nodes from rankTree 2 to rankTree 1.
// Need to modify the pointers to local tree node from r2 to the node from r1
// because we don't update the node pointer of each leaf in r2. Only the root of
// each rankTree stores the correct pointer to its local tree node
inline std::vector<rankTree*> rankTree::Merge(std::vector<rankTree*>& r1, std::vector<rankTree*>& r2, void* node) {
  std::vector<rankTree*> Seq;
  Seq.reserve(r1.size() + r2.size());
  size_t p = 0, q = 0;
  while (p < r1.size() || q < r2.size()) {
    if (p == r1.size()) {
      while (q < r2.size()) {
        r2[q]->Node = node;
        r2[q]->parent = nullptr;
        Seq.push_back(r2[q]);
        q++;
      }
      break;
    }
    if (q == r2.size()) {
      while (p < r1.size()) {
        r1[p]->Node = node;
        r1[p]->parent = nullptr;
        Seq.push_back(r1[p]);
        p++;
      }
      break;
    }
    if (r1[p]->rank > r2[q]->rank) {
      r2[q]->Node = node;
      r2[q]->parent = nullptr;
      Seq.push_back(r2[q]);
      q++;
    } else {
      r1[p]->Node = node;
      r1[p]->parent = nullptr;
      Seq.push_back(r1[p]);
      p++;
    }
  }
  return build(Seq, node);
}
// inline std::bitset<64> rankTree::updateBitMap(std::vector<rankTree*>& rTrees) {
//   std::bitset<64> Ans;
//   Ans.reset();
//   for (auto it : rTrees)
//     Ans |= it->edgemap;
//   return Ans;
// }

// Update bitmap from leaf to root
// If no update need, we return nullptr, otherwise we return the root of the rankTree
// Which means we need go to level - 1 to keep updating
inline rankTree* rankTree::updateBitMap(rankTree* r, bool nval, size_t l) {
  r->edgemap[l] = nval;
  while (r->parent) {
    bool oldval = r->parent->edgemap[l];
    bool lval = (r->parent->lchild != nullptr) ? r->parent->lchild->edgemap[l] : false;
    bool rval = (r->parent->rchild != nullptr) ? r->parent->rchild->edgemap[l] : false;
    // no need to go up
    if (oldval == (lval | rval)) return nullptr;
    r->parent->edgemap[l] = nval;
    r = r->parent;
  }
  // r exp to be the root of the rankTree.
  return r;
}
inline rankTree* rankTree::getNextLeaf(rankTree* p) {
  if (!p || !p->parent) return nullptr;
  if (p == p->parent->lchild && p->parent->rchild)
    return getLeftMostLeaf(p->parent->rchild);
  else
    return getNextLeaf(p->parent);
  return nullptr;
}
inline rankTree* rankTree::getLeftMostLeaf(rankTree* p) {
  if (!p->lchild && !p->rchild) return p;
  while (p->lchild || p->rchild)
    p = (p->lchild) ? p->lchild : p->rchild;
  return p;
}