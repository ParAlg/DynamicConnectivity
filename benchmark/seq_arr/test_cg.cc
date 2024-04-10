#include "cluster_graph.hpp"
#include <parlay/primitives.h>
#include <parlay/sequence.h>

int main() {
  size_t n = 5, m = 10;
  cluster_graph A(0, 0, 0);
  cluster_graph B(0, 0, 0);
  A.testMakeArray(n);
  A.printNodeSize(n);
  auto ch1 = new cluster_graph::child(nullptr, nullptr, 0, 2);
  // A.insertChild(ch1);
  A.printNodeSize(n + 1);
  // A.deleteChild(ch1);
  A.printNodeSize(n);
  // A.testMakeBitMap(n);
  B.testMakeArray(m);
  B.printNodeSize(m);
  auto ch2 = new cluster_graph::child(nullptr, nullptr, 0, 9);
  // B.insertChild(ch2);
  B.printNodeSize(m + 1);
  // B.deleteChild(ch2);
  B.printNodeSize(m);
  // B.testMakeBitMap(m);
  return 0;
}