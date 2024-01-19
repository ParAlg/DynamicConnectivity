#include <parlay/primitives.h>

template <class LocalTree>
class Cluster;

template <class LocalTree>
class Leaf {
  using cluster = Cluster<LocalTree>;
  Leaf() { _type = 1; }

  // TODO
  std::vector<cluster*> fully_traverse() {
    return std::vector<cluster*>();
  }

 public:
  cluster* _parent;  // the parent of this leaf (i.e. vertex).
  uint8_t _type;
};

// A cluster is an internal node in the cluster forest. A local tree
// is a data structure storing the set of children of the cluster.
template <class LocalTree>
class Cluster {
 public:
  // node_t is the maximum number of nodes.
  using node_t = uint32_t;
  using edge_t = std::pair<node_t, node_t>;

//  static traverse_from_leaf(Leaf<LocalTree>* leaf) {}

  Cluster() { _type = 1; }

  // Some debugging / invariant-checking routines.
  node_t MaxSize() const {
    return 1 << level;   // max size for level i is 2^i.
  }

  void CheckInvariant() const {
    assert(_type == 1);
    // TODO
  }

  // The number of leaves in this cluster.
  node_t Size() const { return _size; }

  uint8_t level() const { return _level; }

  // Returns the set bits of this cluster. Useful when merging.
  uint8_t GetBits() const { return _bits; }

  // TODO: how to "get" a level i edge, and then get the next one? Is
  // this even the right approach? To discuss.
  edge_t remove_level_i_edge() {}

  // Ensures that this cluster is incident to a blocked edge.
  void RepairInvariant() {
    // TODO.
  }

  // The cluster is in an invalid state after this point and should be
  // cleaned up by the caller of this function.
  LocalTree&& GetLocalTree() { return std::move(_lt); }

  // Unlink this node from its parent (if it exists), and
  // clear/relinquish all memory.
  void Free() {
    // TODO.
  }

  // Merge with the sibling cluster.
  void Merge(Cluster<LocalTree>* sibling) {
    // Our level is the larger one.
    assert(level() >= sibling->level);
    assert(Size() + sibling->size() <= MaxSize());

    // Update our size.
    _size += sibling->Size();
    // Extract the sibling's local-tree, and merge.
    _bits |= sibling->Bits();
    _lt.Merge(sibling->GetLocalTree());
    sibling->Free();
  }

  // Returns nullptr if this is a root cluster. Otherwise, returns a
  // pointer to the parent of this cluster.
  Cluster<LocalTree>* get_parent() const { return _parent; }

  // Returns true iff there is a level i edge incident to some leaf in
  // this cluster.
  bool has_level_i_edge(node_t i) const { return _bits & (1 << i); }

 private:
  Cluster<LocalTree>* _parent;   // the parent cluster of this cluster.
  uint8_t _type;      // 1 if an internal node, 0 if a leaf.
  uint8_t _level;     // a value between [1, log(n)].
  node_t _size;       // the number of leaves contained in this cluster.

  // Bits indicating whether this cluster has a level-i edge.
  node_t _bits;

  LocalTree _lt;   // the LocalTree object storing the children of this cluster.

  // Used internally when this node recieves a new level i edge in its
  // cluster.
  void set_level_i_bit(node_t level) const { _bits = _bits & (1 << level); }
};

class ArrayLocalTree {
 public:
 private:
};
