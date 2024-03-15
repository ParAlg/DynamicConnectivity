#pragma once
#include <pam/pam.h>
#include <bitset>
struct endpoint_entry {
  using key_t = size_t;
  static bool comp(key_t a, key_t b) { return a < b; }
};
struct edgeset_entry {
  using key_t = size_t;
  using val_t = pam_set<endpoint_entry>;
  static bool comp(key_t a, key_t b) { return a < b; }
};
using EP = pam_set<endpoint_entry>;
using E_ = pam_map<edgeset_entry>;
class leaf {
 public:
  leaf(size_t _id = 0) : E(), parent(nullptr), edgemap(), size(0), id(_id) {}
  bool insert(size_t e, size_t l);
  bool remove(size_t e, size_t l);
  size_t getLevel(size_t e);

 private:
  E_ E;
  void *parent;
  std::bitset<64> edgemap;
  size_t id;
  size_t size;
};
inline bool leaf::insert(size_t e, std::size_t l) {
  auto &E = this->E;
  size++;
  auto it = E.find(l);
  if (it.has_value()) {

  } else {
    EP ep;
    ep.insert(e);
    E.insert(std::make_pair(l, std::move(ep)));
    this->edgemap[l] = 1;
  }
  return false;
}
inline bool leaf::remove(size_t e, size_t l) {
  return false;
}
inline size_t leaf::getLevel(size_t e) {
  return 0;
}