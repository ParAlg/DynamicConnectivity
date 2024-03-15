#include <iostream>
#include <set>
#include <map>
typedef enum { VERTEX, NODE } MODE;
inline size_t setBit(size_t n, size_t p, size_t b) {
  return ((n & (~(1 << p))) | (b << p));
}
inline bool checkSet(size_t n, size_t p) {
  return ((n >> (p - 1)) & 1);
}