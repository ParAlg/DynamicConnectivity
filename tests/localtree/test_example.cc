#include <cmath>
#include <dycon/localTree/SCCWN.hpp>
#include <fstream>
#include <iostream>
void test1() {
  size_t n = 128;
  SCCWN F(n);
  F.lmax = std::log2(n) + 1;
  parlay::sequence<std::pair<size_t, size_t>> e;
  for (size_t i = 1; i < n - 1; i++)
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
  auto del = ins;
  auto db1 = parlay::random_shuffle(del);
  std::cout << "start delete\n";
  for (size_t i = 0; i < db1.size(); i++) {
    F.remove(db1[i].first, db1[i].second);
  }
  //   auto ib1 = parlay::random_shuffle(db1);
  //   for (size_t i = 0; i < ib1.size(); i++)
  //     F.insert(ib1[i].first, ib1[i].second);
  //   for (size_t i = 0; i < ins.size(); i++)
  //     F.remove(ins[i].first, ins[i].second);
}
void test2() {
  SCCWN F(8);
  // F.test_fetch();
}
int main() {
  test1();
  // test2();
  return 0;
}