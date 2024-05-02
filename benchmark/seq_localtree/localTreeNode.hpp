#include "rankTree.hpp"
#include "leaf.hpp"
#include <vector>
#include <stack>
#include <cmath>
class localTreeNode {
 public:
  localTreeNode(rankTree* r, size_t l, size_t n = 1) : edgemap(), level(l), n_v(n), parent(nullptr) {
    rTrees.push_back(r);
  }
  localTreeNode(size_t l, size_t n, std::bitset<64> emap = std::bitset<64>()) :
      edgemap(emap), level(l), n_v(n), parent(nullptr) {}
  void setParent(rankTree* p) { parent = p; }
  rankTree* getParent() { return parent; }
  static localTreeNode* getRoot(leaf* v);
  static localTreeNode* getLevelNode(leaf* v, size_t l);
  static void Merge(localTreeNode* Cu, localTreeNode* Cv);
  static localTreeNode* makeUp(localTreeNode* C, size_t level);
  static localTreeNode* getParent(localTreeNode* U);
  size_t getLevel() { return level; }
  size_t getClusterSize() { return n_v; }
  using stackItem = std::pair<std::vector<rankTree*>::iterator, rankTree*>;
  static leaf* initSearchStack(std::stack<stackItem>& S, localTreeNode* Node, size_t edgeLevel, size_t maxLevel);
  static leaf* getIncidentLeaf(std::queue<localTreeNode*>& Q, std::stack<stackItem>& S, size_t edgeLevel,
                               size_t maxLevel);
  static leaf* updateSearchStack(std::stack<stackItem>& S, size_t edgeLevel, size_t maxLevel);
  static leaf* getLeafFromNode(localTreeNode* p) {
    rankTree* T = static_cast<rankTree*>(*(p->rTrees.begin()));
    return static_cast<leaf*>(T->getLeaf());
  }
  static void updateBitMap(leaf* u, bool oval, bool nval, size_t level);
  static void removeChild(localTreeNode* F, localTreeNode* S);
  static void updateGrandParent(localTreeNode* GP, localTreeNode* P, localTreeNode* PSib);

  int get_cluster_graph_size();

  int64_t space() {
    int64_t space = 0;
    space += sizeof(std::vector<rankTree*>);
    space += rTrees.size() * sizeof(rankTree*);
    space += sizeof(std::bitset<64>);
    space += 2*sizeof(size_t);
    space += sizeof(rankTree*);
    return space;
  }

 private:
  std::vector<rankTree*> rTrees;
  std::bitset<64> edgemap;
  size_t level;
  size_t n_v;  // number of vertices incident to this cluster node.
  rankTree* parent;
  // for the parent here. We cannot let the parent pointer point to the upper level local tree node.
  // because when we are merging node, we only merge rank trees. The parent will be invalid if we don't also update the
  // children of the cluster node But this introduces extra running time.
};

static int get_cluster_graph_size_dfs(rankTree* node) {
  if (node->leaf) return 1;
  int total = 0;
  if (node->lchild) total += get_cluster_graph_size_dfs(node->lchild);
  if (node->rchild) total += get_cluster_graph_size_dfs(node->rchild);
  return total;
}

int localTreeNode::get_cluster_graph_size() {
  int num_leaves = 0;
  for (auto rank_tree_root : rTrees)
    num_leaves += get_cluster_graph_size_dfs(rank_tree_root);
  return num_leaves;
}

inline localTreeNode* localTreeNode::getRoot(leaf* v) {
  auto rTree = static_cast<rankTree*>(v->getParent());
  auto lTree = static_cast<localTreeNode*>(rankTree::getRoot(rTree)->getNode());
  while (lTree->parent) {
    rTree = static_cast<rankTree*>(lTree->parent);
    lTree = static_cast<localTreeNode*>(rankTree::getRoot(rTree)->getNode());
  }
  return lTree;
}
inline localTreeNode* localTreeNode::getLevelNode(leaf* v, size_t l) {
  auto rTree = static_cast<rankTree*>(v->getParent());
  auto lTree = static_cast<localTreeNode*>(rankTree::getRoot(rTree)->getNode());
  while (lTree->level > l) {
    rTree = static_cast<rankTree*>(lTree->parent);
    lTree = static_cast<localTreeNode*>(rankTree::getRoot(rTree)->getNode());
  }
  return lTree;
}
inline void localTreeNode::Merge(localTreeNode* Cu, localTreeNode* Cv) {
  Cu->n_v = Cu->n_v + Cv->n_v;
  Cu->rTrees = rankTree::Merge(Cu->rTrees, Cv->rTrees, static_cast<void*>(Cu));
  // Cu->edgemap = rankTree::updateBitMap(Cu->rTrees);
  delete Cv;
}

// inline void localTreeNode::Merge(localTreeNode* Cu, localTreeNode* Cv) {
//   // Make up the difference of level
//   if (Cu->level > Cv->level)
//     Cv = makeUp(Cv, Cu->level - Cv->level);
//   else if (Cu->level < Cv->level)
//     Cu = makeUp(Cu, Cv->level - Cu->level);
//   if (Cu->level == Cv->level && Cu->n_v + Cv->n_v > (2 << Cu->level)) {
//     // If merge Cu and Cv violate the invirant 2^level, we get new level+1 cluster node
//     auto P = new localTreeNode(Cu->level + 1, Cu->n_v + Cv->n_v);
//     auto r1 = new rankTree(std::log2(Cu->n_v), static_cast<void*>(P), static_cast<void*>(Cu), Cu->edgemap);
//     auto r2 = new rankTree(std::log2(Cv->n_v), static_cast<void*>(P), static_cast<void*>(Cv), Cv->edgemap);
//     Cu->parent = r1;
//     Cv->parent = r2;
//     std::vector<rankTree*> v1, v2;
//     v1.push_back(r1);
//     v2.push_back(r2);
//     P->rTrees = rankTree::Merge(v1, v2, P);
//     P->edgemap = rankTree::updateBitMap(P->rTrees);
//   } else {
//     Cu->n_v = Cu->n_v + Cv->n_v;
//     Cu->rTrees = rankTree::Merge(Cu->rTrees, Cv->rTrees, static_cast<void*>(Cu));
//     Cu->edgemap = rankTree::updateBitMap(Cu->rTrees);
//     delete Cv;
//   }

//   // merge all children of Cv to Cu
//   // Nu = Nu + Nv
//   // Nv = 0;
//   // Merge rank trees of Cv to rank trees of Cu
//   // Tricky part here, we need to modify the local tree node pointer in each rank tree
//   // But we cannot traversal all the leaf in each rank tree.
//   // So we only change the node of root of each rank tree
//   // So, although every node in rank tree has a local tree pointer.
//   // Only the pointer in root of each rank tree will save the correct information
//   // The edgemap will be updated after merging all the rank trees.
//   // Delete Cv
// }
inline localTreeNode* localTreeNode::makeUp(localTreeNode* C, size_t level) {
  while (level) {
    // generate a parent for local tree node c
    // allocate a new local tree node and a new rank tree node.
    // The local tree node should have level C->l+1,n_v and edgemap should be the same.
    auto P = new localTreeNode(C->level - 1, C->n_v, C->edgemap);
    // the rank of the rank tree should be the log2(C->n_v)
    // we don't know how many multigraph nodes inside C, so we cannot simply assign the rank
    // of rank trees in C to C's parent's rank tree.
    auto r = new rankTree(std::log2(C->n_v));
    // rank tree has two direction pointers : leaf(another local tree node) , node (to the local tree node containing
    // itself)
    // let the node pointer of the rank tree point to new allocated local tree node p
    r->setNode(static_cast<void*>(P));
    // the rank tree has only one node and its leaf should point to C
    r->setLeaf(static_cast<void*>(C));
    // let this sole rank tree be the parent of C;
    C->parent = r;
    // For C's parent, it should contain a sole rank tree with only one node which point to C
    // Because we makeup the level difference and no other vertices will take participant in this.
    // We only need to keep C's incident vertices.
    // C might have multiple rank trees, but C's parent can only have one.
    P->rTrees.push_back(r);
    C = P;
    level--;
  }
  return C;
}
// If initSearchStack and updateSearchStack both empty, then Cu has no remaining level i edges.
inline leaf* localTreeNode::initSearchStack(std::stack<stackItem>& S, localTreeNode* Node, size_t edgeLevel,
                                            size_t maxLevel) {
  while (Node->level < maxLevel) {
    // find a rank tree that has edgeLevel incident edge
    auto it = Node->rTrees.begin();
    while (it != Node->rTrees.end()) {
      rankTree* T = *it;
      if (T->checkLevelEdge(edgeLevel)) break;
      it++;
    }
    if (it == Node->rTrees.end()) return nullptr;
    rankTree* rleaf = rankTree::getLeftMostLeaf(static_cast<rankTree*>(*it));
    while (rleaf && !rleaf->checkLevelEdge(edgeLevel))
      rleaf = rankTree::getNextLeaf(rleaf);
    if (!rleaf) return nullptr;
    S.push(std::make_pair(it, rleaf));
    Node = static_cast<localTreeNode*>(rleaf->getLeaf());
    // we didnt push lmax node(which is vertex) into the stack
  }
  ASSERT_MSG(Node->n_v == 1, "rleaf node has multiple vertices");
  leaf* V = getLeafFromNode(Node);
  if (!V->checkLevelEdge(edgeLevel)) return nullptr;
  return V;
}
// When we are finding the incident edge, we need to traverse from level i cluster down to leaf
// But in the stack, we only know which rank tree we are in but no direct information about the level
// which can be used to infer whether we reach the leaf
// So we have a node pointer on the root of each rank tree. We can check the level of that node
inline leaf* localTreeNode::updateSearchStack(std::stack<stackItem>& S, size_t edgeLevel, size_t maxLevel) {
  while (!S.empty()) {
    stackItem P = S.top();
    S.pop();

    // Check if there are any incident edges in other nodes of the same rank tree
    rankTree* rleaf = rankTree::getNextLeaf(P.second);
    while (rleaf && !rleaf->checkLevelEdge(edgeLevel))
      rleaf = rankTree::getNextLeaf(rleaf);
    if (rleaf) {
      S.push(std::make_pair(P.first, rleaf));
      return initSearchStack(S, static_cast<localTreeNode*>(rleaf->getLeaf()), edgeLevel, maxLevel);
    }

    // Check if there are any incident edges in other rank trees within samel local tree node
    auto it = P.first;
    rankTree* root = *it;
    auto lNode = static_cast<localTreeNode*>(root->getNode());
    while (it != lNode->rTrees.end()) {
      rankTree* rTree = *it;
      if (!rTree->checkLevelEdge(edgeLevel)) it++;
    }
    if (it != lNode->rTrees.end()) {
      rankTree* rleaf = rankTree::getLeftMostLeaf(static_cast<rankTree*>(*it));
      while (rleaf && !rleaf->checkLevelEdge(edgeLevel))
        rleaf = rankTree::getNextLeaf(rleaf);
      ASSERT_MSG(rleaf != nullptr, "bitmap incorrect");
      S.push(std::make_pair(it, rleaf));
      return initSearchStack(S, static_cast<localTreeNode*>(rleaf->getLeaf()), edgeLevel, maxLevel);
      ;
    }
    // This cluster node has been explored. Pop the stack to i-1 level.
  }
  return nullptr;
}
// Check if there are any incident edges in other nodes of the same rank tree
// rankTree* rleaf = rankTree::getNextLeaf(P.second);
// while (rleaf && !rleaf->checkLevelEdge(edgeLevel))
//   rleaf = rankTree::getNextLeaf(rleaf);
// if (rleaf) {
//   leaf* V = getLeafFromNode(static_cast<localTreeNode*>(rleaf->getLeaf()));
//   S.push(std::make_pair(P.first, rleaf));
//   return V;
// }

// Check if there are any incident edges in other rank trees within samel local tree node
// auto it = P.first;
// rankTree* root = *it;
// auto lNode = static_cast<localTreeNode*>(root->getNode());
// while (it != lNode->rTrees.end()) {
//   rankTree* rTree = *it;
//   if (!rTree->checkLevelEdge(edgeLevel)) it++;
// }
// if (it != lNode->rTrees.end()) {
//   rankTree* rleaf = rankTree::getLeftMostLeaf(static_cast<rankTree*>(*it));
//   while (rleaf && !rleaf->checkLevelEdge(edgeLevel))
//     rleaf = rankTree::getNextLeaf(rleaf);
//   ASSERT_MSG(rleaf != nullptr, "bitmap incorrect");
//   S.push(std::make_pair(it, rleaf));
//   leaf* V = getLeafFromNode(static_cast<localTreeNode*>(rleaf->getLeaf()));
//   return V;
// }
// return nullptr;
// }
inline leaf* localTreeNode::getIncidentLeaf(std::queue<localTreeNode*>& Q, std::stack<stackItem>& S, size_t edgeLevel,
                                            size_t maxLevel) {
  leaf* Leaf = updateSearchStack(S, edgeLevel, maxLevel);
  while (!Leaf) {
    if (Q.empty()) return nullptr;
    localTreeNode* Node = Q.front();
    Q.pop();
    while (!S.empty())
      S.pop();
    Leaf = initSearchStack(S, Node, edgeLevel, maxLevel);
  }
  return Leaf;
}
inline void localTreeNode::updateBitMap(leaf* u, bool oval, bool nval, size_t l) {
  auto rTree = static_cast<rankTree*>(u->getParent());
  while (rTree) {
    rTree = rankTree::updateBitMap(rTree, nval, l);
    if (!rTree) return;
    rTree = static_cast<localTreeNode*>(rTree->getNode())->parent;
  }
}
inline localTreeNode* localTreeNode::getParent(localTreeNode* U) {
  if (!U->parent) return nullptr;
  auto rTree = rankTree::getRoot(U->parent);

  return static_cast<localTreeNode*>((rTree->getRoot(rTree))->getNode());
}
inline void localTreeNode::removeChild(localTreeNode* F, localTreeNode* S) {
  F->rTrees = rankTree::remove(S->parent, F);
  F->n_v -= S->n_v;
}
inline void localTreeNode::updateGrandParent(localTreeNode* GP, localTreeNode* P, localTreeNode* PSib) {
  rankTree* r1 = new rankTree(std::log2(P->getClusterSize()));
  rankTree* r2 = new rankTree(std::log2(PSib->getClusterSize()));
  std::vector<rankTree*> Seq;
  r1->setLeaf(static_cast<void*>(P));
  r2->setLeaf(static_cast<void*>(PSib));
  if (r1->getRank() == r2->getRank()) {
    rankTree* r3 = new rankTree(r1, r2);
    Seq.push_back(r3);
  } else {
    Seq.push_back(r1);
    Seq.push_back(r2);
  }
  GP->rTrees = rankTree::Merge(GP->rTrees, Seq, GP);
}