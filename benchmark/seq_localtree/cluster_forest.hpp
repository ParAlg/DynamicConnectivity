#include <stack>
#include <queue>
#include <unordered_set>
#include "localTreeNode.hpp"
class cluster_forest {
 public:
  cluster_forest(size_t _n);
  void insert(size_t u, size_t v);
  bool is_connected(size_t u, size_t v) { return (localTreeNode::getRoot(V[u]) == localTreeNode::getRoot(V[v])); }
  void remove(size_t u, size_t v);
  // Statistic gathering helpers
  void print_cg_sizes();

 private:
  // std::vector<localTree*> CC;
  std::vector<leaf*> V;
  size_t n;
  // Helper functions
  size_t size_const(size_t level);
};

void cluster_forest::print_cg_sizes() {
  size_t total = 0;
  size_t num_cg = 0;
  size_t max = 0;
  std::set<localTreeNode*> visited_cg;
  for (size_t i = 0; i < n; i++) {
    auto rTree = static_cast<rankTree*>(V[i]->getParent());
    while (rTree) {
      auto cg = static_cast<localTreeNode*>(rankTree::getRoot(rTree)->getNode());
      if (visited_cg.find(cg) == visited_cg.end()) {
        size_t size = cg->get_cluster_graph_size();
        total += size;
        num_cg += 1;
        max = std::max(max, size);
        visited_cg.insert(cg);
      }
      rTree = static_cast<rankTree*>(cg->getParent());
    }
  }
  std::cout << "Cluster Graph Sizes:" << std::endl;
  std::cout << "Avg: " << (float)total/(float)num_cg << " Max: " << max << std::endl;
}

inline cluster_forest::cluster_forest(size_t _n) : n(_n) {
  V.resize(n);
  // for (auto &it : V) {
  for (size_t i = 0; i < V.size(); i++) {
    // 3 new, 4 assignment
    V[i] = new leaf(i);
    auto p = new rankTree();

    V[i]->linkToRankTree(static_cast<void *>(p));    // Leaf link to rank tree
    p->setLeaf(static_cast<void *>(V[i]));           // rank tree link to leaf
    auto node = new localTreeNode(p, std::log2(n));  // add rank tree to local tree
    p->setNode(static_cast<void *>(node));           // rank tree link to local tree node
    node = localTreeNode::makeUp(node, std::log2(n));
  }
  std::cout << "n=" << n << std::endl; 
}

inline void cluster_forest::insert(size_t u, size_t v) {
  // Insert the edge at the top level
  auto Cu = localTreeNode::getRoot(V[u]);
  auto Cv = localTreeNode::getRoot(V[v]);
  if (V[u]->insert(v, 0)) localTreeNode::updateBitMap(V[u], 0, 1, 0);
  if (V[v]->insert(u, 0)) localTreeNode::updateBitMap(V[v], 0, 1, 0);
  if (Cu != Cv) localTreeNode::Merge(Cu, Cv);
  // Repeatedly push it down until it is blocked
  size_t i = 1;
  Cu = localTreeNode::getLevelNode(V[u], i);
  Cv = localTreeNode::getLevelNode(V[v], i);
  while(Cu->getClusterSize() + Cv->getClusterSize() <= size_const(i)) {
    if (Cu != Cv) localTreeNode::Merge(Cu, Cv);
    if (V[u]->remove(v, i-1)) localTreeNode::updateBitMap(V[u], 1, 0, i-1);
    if (V[u]->insert(v, i)) localTreeNode::updateBitMap(V[u], 0, 1, i);
    if (V[v]->remove(u, i-1)) localTreeNode::updateBitMap(V[v], 1, 0, i-1);
    if (V[v]->insert(u, i)) localTreeNode::updateBitMap(V[v], 0, 1, i);
    i++;
    Cu = localTreeNode::getLevelNode(V[u], i);
    Cv = localTreeNode::getLevelNode(V[v], i);
  }
}

// case 2
// Get parent of Cu
// Get grandparent of Cu
// Remove parent from grandparent
// Create a sibling of parent
// Create a new level i+1 cluster New
// We scan the vectors of the explorered edge
// get the level i+1 cluster, remove it from parent, merge it with New
// remove the edge from i,check the bitmap
// insert the edge to i+1,check the bitmap
// let the sibling point to the New
// Create localtree node Lnew containing a rank tree to the parent and its sibling
// Merge gp and lnew
// case 1
inline void cluster_forest::remove(size_t u, size_t v) {
  size_t i = V[u]->getSize() < V[v]->getSize() ? V[u]->getLevel(v) : V[v]->getLevel(u);
  V[u]->remove(v, i);
  V[v]->remove(u, i);

  while (i >= 0) {
    auto Cu = localTreeNode::getLevelNode(V[u], i + 1);
    auto Cv = localTreeNode::getLevelNode(V[v], i + 1);
    std::queue<localTreeNode *> Qu, Qv;
    Qu.push(Cu);
    Qv.push(Cv);

    using searchStack = std::stack<std::pair<std::vector<rankTree *>::iterator, rankTree *>>;
    searchStack Su, Sv;
    leaf *Lv = nullptr;
    leaf *Lu = nullptr;
    std::set<size_t>::iterator Uit, Ued, Vit, Ved;

    std::vector<std::pair<leaf *, size_t>> Eu, Ev;  // searched edge
    Eu.reserve(n / (1 << Cu->getLevel()));
    Ev.reserve(n / (1 << Cv->getLevel()));

    std::unordered_set<localTreeNode *> Expu, Expv;
    size_t Nu = 0;
    size_t Nv = 0;
    while (true) {
      // parallel search on Cu
      {
        if (!Lu) {
          Lu = localTreeNode::getIncidentLeaf(Qu, Su, i, std::log2(n));
          if (!Lu) {
            if (Nu < Nv) {

              for (auto e : Eu) {
                auto a = localTreeNode::getLevelNode(e.first, i + 1);
                auto b = localTreeNode::getLevelNode(V[e.second], i + 1);
                if (a != Cu) localTreeNode::Merge(Cu, a);
                if (b != Cu) localTreeNode::Merge(Cu, b);
                if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
                if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
                if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
                if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
              }
              auto P = localTreeNode::getParent(Cu);
              auto GP = localTreeNode::getParent(P);
              localTreeNode::removeChild(P, Cu);
              if (GP) localTreeNode::removeChild(GP, P);

              auto rTree = new rankTree(std::log2(Cu->getClusterSize()));
              rTree->setLeaf(static_cast<void *>(Cu));
              Cu->setParent(rTree);
              auto PSib = new localTreeNode(rTree, P->getLevel(), Cu->getClusterSize());
              ASSERT_MSG(PSib->getLevel() == i, "P is not the father of Cu");
              if (GP) localTreeNode::updateGrandParent(GP, P, PSib);
            } else {
              for (auto e : Ev) {
                auto a = localTreeNode::getLevelNode(e.first, i + 1);
                auto b = localTreeNode::getLevelNode(V[e.second], i + 1);
                if (a != Cv) localTreeNode::Merge(Cv, a);
                if (b != Cv) localTreeNode::Merge(Cv, b);
                if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
                if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
                if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
                if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
              }
              auto P = localTreeNode::getParent(Cv);
              auto GP = localTreeNode::getParent(P);
              localTreeNode::removeChild(P, Cv);
              if (GP) localTreeNode::removeChild(GP, P);

              auto rTree = new rankTree(std::log2(Cv->getClusterSize()));
              rTree->setLeaf(static_cast<void *>(Cv));
              Cu->setParent(rTree);
              auto PSib = new localTreeNode(rTree, P->getLevel(), Cv->getClusterSize());
              ASSERT_MSG(PSib->getLevel() == i, "P is not the father of Cv");
              if (GP) localTreeNode::updateGrandParent(GP, P, PSib);
            }
            // no more edge to explorer
            //  End and push
            break;
          }
          auto it = Lu->getLevelIterator(i);
          Uit = it.first;
          Ued = it.second;
        }

        auto Cuv = localTreeNode::getLevelNode(V[*Uit], i + 1);
        if (Expu.find(Cuv) == Expu.end()) {
          Expu.insert(Cuv);
          Nu += Cuv->getClusterSize();
          Qu.push(Cuv);
        }
        if (Expv.find(Cuv) != Expv.end()) {
          if (Nu < Nv) {
            for (auto e : Eu) {
              auto C = localTreeNode::getLevelNode(e.first, i + 1);
              localTreeNode::Merge(Cu, C);
              if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
              if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
              if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
              if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
            }
          } else {
            for (auto e : Ev) {
              auto C = localTreeNode::getLevelNode(e.first, i + 1);
              localTreeNode::Merge(Cv, C);
              if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
              if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
              if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
              if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
            }
          }
          // find a replacement edge
          return;
        }
        Eu.push_back(std::pair<leaf *, size_t>(Lu, *Uit));
        Uit++;
        if (Uit == Ued) Lu = nullptr;
      }
      // Parallel search on Cv
      {
        if (!Lv) {
          Lv = localTreeNode::getIncidentLeaf(Qv, Sv, i, std::log2(n));
          if (!Lv) {
            if (Nu < Nv) {

              for (auto e : Eu) {
                auto a = localTreeNode::getLevelNode(e.first, i + 1);
                auto b = localTreeNode::getLevelNode(V[e.second], i + 1);
                if (a != Cu) localTreeNode::Merge(Cu, a);
                if (b != Cu) localTreeNode::Merge(Cu, b);
                if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
                if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
                if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
                if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
              }
              auto P = localTreeNode::getParent(Cu);
              auto GP = localTreeNode::getParent(P);
              localTreeNode::removeChild(P, Cu);
              if (GP) localTreeNode::removeChild(GP, P);

              auto rTree = new rankTree(std::log2(Cu->getClusterSize()));
              rTree->setLeaf(static_cast<void *>(Cu));
              Cu->setParent(rTree);
              auto PSib = new localTreeNode(rTree, P->getLevel(), Cu->getClusterSize());
              ASSERT_MSG(PSib->getLevel() == i, "P is not the father of Cu");
              if (GP) localTreeNode::updateGrandParent(GP, P, PSib);
            } else {
              for (auto e : Ev) {
                auto a = localTreeNode::getLevelNode(e.first, i + 1);
                auto b = localTreeNode::getLevelNode(V[e.second], i + 1);
                if (a != Cv) localTreeNode::Merge(Cv, a);
                if (b != Cv) localTreeNode::Merge(Cv, b);
                if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
                if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
                if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
                if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
              }
              auto P = localTreeNode::getParent(Cv);
              auto GP = localTreeNode::getParent(P);
              localTreeNode::removeChild(P, Cv);
              if (GP) localTreeNode::removeChild(GP, P);

              auto rTree = new rankTree(std::log2(Cv->getClusterSize()));
              rTree->setLeaf(static_cast<void *>(Cv));
              Cu->setParent(rTree);
              auto PSib = new localTreeNode(rTree, P->getLevel(), Cv->getClusterSize());
              ASSERT_MSG(PSib->getLevel() == i, "P is not the father of Cv");
              if (GP) localTreeNode::updateGrandParent(GP, P, PSib);
            }
            // no more edge to explorer
            //  End and push
            break;
          }
          auto it = Lv->getLevelIterator(i);
          Vit = it.first;
          Ved = it.second;
        }

        auto Cuv = localTreeNode::getLevelNode(V[*Vit], i + 1);
        if (Expv.find(Cuv) == Expv.end()) {
          Expv.insert(Cuv);
          Nv += Cuv->getClusterSize();
          Qv.push(Cuv);
        }
        if (Expu.find(Cuv) != Expu.end()) {
          if (Nu < Nv) {
            for (auto e : Eu) {
              auto C = localTreeNode::getLevelNode(e.first, i + 1);
              localTreeNode::Merge(Cu, C);
              if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
              if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
              if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
              if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
            }
          } else {
            for (auto e : Ev) {
              auto C = localTreeNode::getLevelNode(e.first, i + 1);
              localTreeNode::Merge(Cv, C);
              if (e.first->remove(e.second, i)) localTreeNode::updateBitMap(e.first, 1, 0, i);
              if (e.first->insert(e.second, i + 1)) localTreeNode::updateBitMap(e.first, 0, 1, i + 1);
              if (V[e.second]->remove(e.first->getID(), i)) localTreeNode::updateBitMap(V[e.second], 1, 0, i);
              if (V[e.second]->insert(e.first->getID(), i + 1)) localTreeNode::updateBitMap(V[e.second], 0, 1, i + 1);
            }
          }
          // find a replacement edge
          return;
        }
        Ev.push_back(std::pair<leaf *, size_t>(Lv, *Vit));
        Vit++;
        if (Vit == Ved) Lv = nullptr;
      }
    }
    i--;
  }
  // two stacks for parallel search
  // two vectors for edges searched in parallel search
  // two queues for the explorered cluster node, saved for further explorer
  // one hashtable for test if reach a common  cluster node.
  // each search, get one incident edge
  // add edge to corresponding vector
  // check if the incident edge connects to a new node.
  // if not, move to next parallel search
  // if so, check if the new node has been reached by another search.
  // if not, update the hashtable and queue
  // if so, we find a replacement edge. Decide which edges need to be pushed.
  // If we reach the two case. Decide which edges need to be pushed.
  // If no replacement edge found, go back to i-1 level.
}

size_t cluster_forest::size_const(size_t level) {
  return n/(1<<level);
}
