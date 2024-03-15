#include "assert.hpp"
#include "cluster_forest.hpp"
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <iostream>
#include <set>
#include <map>
#include <vector>
#include <bitset>
#include <queue>
#include <deque>
int main() {
  cluster_forest G(10);
  G.insert(1, 2);
  G.insert(2, 3);
  G.insert(4, 5);
  G.insert(3, 6);
  G.insert(6, 7);
  G.insert(8, 9);
  ASSERT_MSG(G.is_connected(1, 3) == true, "should connect");
  ASSERT_MSG(G.is_connected(1, 4) == false, "should not connect");
  ASSERT_MSG(G.is_connected(5, 4) == true, "should connect");
  ASSERT_MSG(G.is_connected(6, 4) == false, "should not connect");
  ASSERT_MSG(G.is_connected(1, 6) == true, "should connect");
  ASSERT_MSG(G.is_connected(1, 6) == true, "should connect");
  return 0;
}