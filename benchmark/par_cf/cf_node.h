#include <parlay/primitives.h>

template <class LocalTree>
class cluster {
 public:
  // node_t is the maximum number of nodes.
  using node_t = uint32_t;
  using edge_t = std::pair<node_t, node_t>;

  // Some debugging / invariant-checking routines.
  node_t MaxSize() const {
    return 1 << level;   // max size for level i is 2^i.
  }

  // The number of leaves in this cluster.
  node_t Size() const {
    return _size;
  }

  uint8_t level() const {
    return _level;
  }

  // TODO: how to "get" a level i edge, and then get the next one?
  edge_t remove_level_i_edge() {

  }

  // Ensures that this cluster is incident to a blocked edge.
  void RepairInvariant() {

  }

  // Merge with the sibling cluster. We know that
  void Merge(cluster* sibling) {
    assert(level() == sibling->level);
    assert(Size() + sibling->size() <= MaxSize());
  }


  // Returns nullptr if this is a root cluster. Otherwise, returns a
  // pointer to the parent of this cluster.
  cluster* get_parent() const { return _parent; }

  // Returns true iff there is a level i edge incident to some leaf in
  // this cluster.
  bool has_level_i_edge(node_t i) const {
    return _bits & (1 << i);
  }

 private:
  uint8_t _level;   // a value between [1, log(n)].
  node_t _size;     // the number of leaves contained in this cluster.

  // Bits indicating whether this cluster has a level-i edge.
  node_t _bits;

  cluster* _parent;   // the parent cluster of this cluster.
  LocalTree _lt;   // the LocalTree object storing the children of this cluster.

  // Used internally when this node recieves a new level i edge in its
  // cluster.
  void set_level_i_bit(node_t level) const {
    _bits = _bits & (1 << level);
  }

};
