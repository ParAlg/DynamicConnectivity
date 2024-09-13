#include "parlay/primitives.h"
#include "parlay/random.h"
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <dycon/localTree/SCCWN.hpp>
#include <fstream>
#include <iostream>
void test1() {
  size_t n = 7;
  SCCWN F(n);
  F.lmax = std::log2(n + 1) + 1;
  parlay::sequence<std::pair<size_t, size_t>> e;
  for (size_t i = 0; i < n - 1; i++)
    for (size_t j = i + 1; j <= n - 1; j++)
      e.push_back(std::make_pair(i, j));

  // for (size_t i = 0; i < ins.size(); i++)
  //   std::cout << ins[i].first << " " << ins[i].second << std::endl;
  auto ins = e;
  for (size_t i = 0; i < ins.size(); i++)
    F.insert(ins[i].first, ins[i].second);
  // F.run_stat(".", true);
  //   auto del = ins.cut(0, 500000);
  //   F.remove(3, 6);
  //   F.remove(3, 4);
  //   F.remove(4, 7);
  //   F.remove(2, 14);
  F.DebugPrintNonTreeEdges();
  F.DebugPrintTreeEdges();
  F.DebugHelper1();
  auto del = ins;
  auto db1 = parlay::random_shuffle(del);
  for (size_t i = 0; i < db1.size(); i++) {
    F.remove(db1[i].first, db1[i].second);
    F.DebugPrintNonTreeEdges();
    F.DebugPrintTreeEdges();
    F.DebugHelper1();
  }
  //   auto ib1 = parlay::random_shuffle(db1);
  //   for (size_t i = 0; i < ib1.size(); i++)
  //     F.insert(ib1[i].first, ib1[i].second);
  //   for (size_t i = 0; i < ins.size(); i++)
  //     F.remove(ins[i].first, ins[i].second);
}
void test2() {
  size_t n = 8;
  SCCWN F(n);
  F.lmax = std::log2(n) + 1;
  F.insert(1, 2);
  F.insert(1, 3);
  F.insert(1, 4);
  F.insert(1, 5);
  F.insert(1, 6);
  F.insert(1, 7);
  F.insert(3, 4);
  F.remove(1, 4);
}
int main() {
  test1();
  // test2();
  return 0;
}