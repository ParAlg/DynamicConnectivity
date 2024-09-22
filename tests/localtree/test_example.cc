#include <cmath>
#include <dycon/localTree/SCCWN.hpp>
#include <iostream>
void test1() {
  size_t n = 128;
  SCCWN F(n);
  F.lmax = std::log2(n) + 1;
  parlay::sequence<std::pair<size_t, size_t>> e;
  for (size_t i = 1; i < n - 1; i++)
    for (size_t j = i + 1; j <= n - 1; j++)
      e.push_back(std::make_pair(i, j));

  auto ins = e;
  for (size_t i = 0; i < ins.size(); i++)
    F.insert(ins[i].first, ins[i].second);
  auto del = ins;

  auto db1 = parlay::random_shuffle(del);
  std::cout << "start delete\n";
  for (size_t i = 0; i < db1.size(); i++) {
    // F.testTreeEdge(39, n);
    F.remove(db1[i].first, db1[i].second);
    // if (F.testLowestTreeEdge(1, n) == false) {
    //   std::cout << db1[i].first << " " << db1[i].second << std::endl;
    //   std::abort();
    // }
    // if (F.testsplit(db1[i].first) == false ||
    //     F.testsplit(db1[i].second) == false) {
    //   F.getInfo(db1[i].first, db1[i].second);
    //   std::cout << db1[i].first << " " << db1[i].second << std::endl;
    //   std::abort();
    // }
    // if (F.whynotsplit()) {
    //   F.getInfo(41, 76);
    //   std::cout << db1[i].first << " " << db1[i].second << std::endl;
    //   std::abort();
    // }
  }

  for (auto it : db1) {
    if (F.is_connected(it.first, it.second)) {
      // F.getInfo(it.first, it.second);
      std::abort();
    }
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
void test3() {
  size_t n = 2048;
  SCCWN F(n);
  F.lmax = std::log2(n) + 1;
  parlay::sequence<std::pair<size_t, size_t>> e;
  for (size_t i = 1; i < n - 1; i++)
    for (size_t j = i + 1; j <= n - 1; j++)
      e.push_back(std::make_pair(i, j));

  // for (size_t i = 0; i < ins.size(); i++)
  //   std::cout << ins[i].first << " " << ins[i].second << std::endl;
  // auto ins = e.cut(0, 1000000);
  auto ins = e;
  for (size_t i = 0; i < ins.size(); i++)
    F.insert(ins[i].first, ins[i].second);
  // F.run_stat(".", true);
  auto del = ins.cut(0, 500000);
  auto db1 = parlay::random_shuffle(del);
  for (size_t i = 0; i < db1.size(); i++) {
    F.remove(db1[i].first, db1[i].second);
  }
  auto ib1 = parlay::random_shuffle(db1);
  for (size_t i = 0; i < ib1.size(); i++)
    F.insert(ib1[i].first, ib1[i].second);
  for (size_t i = 0; i < ins.size(); i++)
    F.remove(ins[i].first, ins[i].second);
}
int main() {
  test1();
  // test2();
  // test3();
  return 0;
}