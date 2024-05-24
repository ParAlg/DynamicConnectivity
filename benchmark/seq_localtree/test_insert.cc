#include "SCCWN.hpp"
#include <iostream>
#include <fstream>
void test1() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(1, 3);
  F.insertToBlock(1, 4);
  F.insertToBlock(2, 3);
  F.insertToBlock(3, 4);
  F.insertToBlock(2, 4);
  F.remove(1, 2);
  F.remove(1, 3);
  F.remove(1, 4);
  F.remove(2, 4);
  F.run_stat("./", true);
}
void test2() {  // not compression
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(2, 3);
  F.insertToBlock(4, 5);
  F.insertToBlock(5, 3);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 5);
  F.remove(1, 2);
  F.remove(2, 3);
  F.remove(4, 5);
  F.remove(5, 3);
  F.remove(5, 6);
  F.remove(7, 5);
  F.run_stat("./", true);
}
void test3() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 6);
  F.insertToBlock(1, 5);
  F.insertToBlock(1, 7);
  F.insertToBlock(3, 5);
  F.insertToBlock(1, 8);
  F.insertToBlock(7, 8);
  // F.remove(1, 8);
  // F.remove(3, 4);
  F.remove(1, 3);
  F.run_stat(".", true);
}
void test4() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(1, 3);
  F.remove(1, 2);
  F.remove(1, 3);
  F.run_stat(".", true);
}
void test5() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 8);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 7);
  F.insertToBlock(4, 6);
  F.insertToBlock(9, 10);
  F.insertToBlock(9, 1);
  F.insertToBlock(10, 1);
  F.insertToBlock(11, 2);
  // F.statistic(false, false, true);
  F.insertToBlock(11, 10);
  F.insertToBlock(5, 11);
  F.insertToBlock(12, 2);
  // F.statistic(false, false, true);
}
void test6() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 4);
  F.insertToBlock(4, 2);
  F.remove(1, 2);
  F.remove(1, 3);
  F.remove(4, 2);
  F.remove(5, 4);
  F.run_stat(".", true);
}
void test7() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 6);
  F.insertToBlock(6, 7);
  F.insertToBlock(3, 5);
  F.insertToBlock(1, 8);
  F.insertToBlock(5, 8);
  // F.statistic(false, false, true);
}
void test8() {
  SCCWN F(50);

  F.insertToBlock(1, 2);
  F.insertToBlock(1, 3);
  F.insertToBlock(1, 4);
  F.insertToBlock(2, 3);
  F.insertToBlock(2, 4);
  F.insertToBlock(3, 4);

  F.insertToBlock(5, 6);
  F.insertToBlock(6, 7);
  F.insertToBlock(7, 8);

  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(10, 11);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 13);
  F.insertToBlock(16, 9);
  F.insertToBlock(13, 16);
  // F.statistic(false, false, true);
}
void test9() {
  SCCWN F(50);

  F.insertToBlock(1, 2);
  F.insertToBlock(1, 3);
  F.insertToBlock(1, 4);
  F.insertToBlock(2, 3);
  F.insertToBlock(2, 4);
  F.insertToBlock(3, 4);

  F.insertToBlock(5, 6);
  F.insertToBlock(6, 7);
  F.insertToBlock(7, 8);

  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(10, 11);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 13);
  F.insertToBlock(16, 9);
  F.insertToBlock(15, 16);
  F.remove(1, 2);
  F.remove(1, 3);
  F.remove(1, 4);
  F.remove(2, 3);
  F.remove(2, 4);
  F.remove(3, 4);

  F.remove(5, 6);
  F.remove(6, 7);
  F.remove(7, 8);

  F.remove(1, 5);

  F.remove(9, 10);
  F.remove(11, 12);
  F.remove(10, 11);
  F.remove(13, 14);
  F.remove(15, 13);
  F.remove(16, 9);
  F.remove(15, 16);

  // F.statistic(false, false, true);
}
void test10() {
  SCCWN F(100);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 8);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 7);
  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 16);
  F.insertToBlock(9, 11);
  F.insertToBlock(13, 15);
  F.insertToBlock(9, 13);

  F.insertToBlock(17, 18);
  F.insertToBlock(19, 20);
  F.insertToBlock(21, 22);
  F.insertToBlock(23, 24);
  F.insertToBlock(17, 19);
  F.insertToBlock(21, 23);
  F.insertToBlock(17, 21);
  F.insertToBlock(9, 17);

  F.insertToBlock(1, 9);
  F.insertToBlock(9, 25);
  // F.statistic(false, false, true);
  F.insertToBlock(1, 25);
  // F.statistic(false, false, true);
}
void test11() {
  SCCWN F(100);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 8);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 7);
  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 16);
  F.insertToBlock(9, 11);
  F.insertToBlock(13, 15);
  F.insertToBlock(9, 13);

  F.insertToBlock(17, 18);
  F.insertToBlock(19, 20);
  F.insertToBlock(21, 22);
  F.insertToBlock(23, 24);
  F.insertToBlock(17, 19);
  F.insertToBlock(21, 23);
  F.insertToBlock(17, 21);
  F.insertToBlock(9, 17);

  F.insertToBlock(1, 9);

  // F.statistic(false, false, true);
  F.insertToBlock(1, 25);
  // F.statistic(false, false, true);
}
void test12() {
  SCCWN F(120);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 8);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 7);
  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(9, 11);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 16);
  F.insertToBlock(13, 15);
  F.insertToBlock(9, 13);

  F.insertToBlock(17, 18);
  F.insertToBlock(19, 20);
  F.insertToBlock(21, 22);
  F.insertToBlock(23, 24);
  F.insertToBlock(21, 23);
  F.insertToBlock(17, 20);
  F.insertToBlock(17, 24);

  F.insertToBlock(25, 26);
  F.insertToBlock(27, 28);
  F.insertToBlock(29, 30);
  F.insertToBlock(31, 32);
  F.insertToBlock(25, 27);
  F.insertToBlock(29, 31);
  F.insertToBlock(25, 32);

  F.insertToBlock(1, 9);
  F.insertToBlock(17, 25);
  F.insertToBlock(17, 1);
  // F.statistic(false, false, true);

  F.insertToBlock(33, 34);
  F.insertToBlock(35, 36);
  F.insertToBlock(33, 35);
  F.insertToBlock(1, 33);
  // F.statistic(false, false, true);

  F.insertToBlock(37, 33);
  // F.statistic(false, false, true);
  F.insertToBlock(38, 33);
  // F.statistic(false, false, true);

  F.insertToBlock(39, 40);
  F.insertToBlock(39, 1);
  // F.statistic(false, false, true);

  F.insertToBlock(39, 33);
  // F.statistic(false, false, true);
}
void test13() {
  SCCWN F(120);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 8);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 7);
  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(9, 11);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 16);
  F.insertToBlock(13, 15);
  F.insertToBlock(9, 13);

  F.insertToBlock(17, 18);
  F.insertToBlock(19, 20);
  F.insertToBlock(21, 22);
  F.insertToBlock(23, 24);
  F.insertToBlock(21, 23);
  F.insertToBlock(17, 20);
  F.insertToBlock(17, 24);

  F.insertToBlock(25, 26);
  F.insertToBlock(27, 28);
  F.insertToBlock(29, 30);
  F.insertToBlock(31, 32);
  F.insertToBlock(25, 27);
  F.insertToBlock(29, 31);
  F.insertToBlock(25, 32);
  // F.statistic(false, false, true);

  F.insertToBlock(1, 9);
  F.insertToBlock(17, 25);
  F.insertToBlock(17, 1);

  F.insertToBlock(33, 34);
  F.insertToBlock(35, 36);
  F.insertToBlock(33, 35);
  F.insertToBlock(1, 33);
  // F.statistic(false, false, true);

  F.insertToBlock(37, 33);
  F.insertToBlock(38, 33);
  // F.statistic(false, false, true);

  F.insertToBlock(39, 40);
  F.insertToBlock(39, 1);
  // F.statistic(false, false, true);

  F.insertToBlock(39, 38);
  // F.statistic(false, false, true);
}
void test14() {
  SCCWN F(120);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 8);
  F.insertToBlock(1, 3);
  F.insertToBlock(5, 7);
  F.insertToBlock(1, 5);

  F.insertToBlock(9, 10);
  F.insertToBlock(11, 12);
  F.insertToBlock(9, 11);
  F.insertToBlock(13, 14);
  F.insertToBlock(15, 16);
  F.insertToBlock(13, 15);
  F.insertToBlock(9, 13);

  F.insertToBlock(17, 18);
  F.insertToBlock(19, 20);
  F.insertToBlock(21, 22);
  F.insertToBlock(23, 24);
  F.insertToBlock(21, 23);
  F.insertToBlock(17, 20);
  F.insertToBlock(17, 24);

  F.insertToBlock(25, 26);
  F.insertToBlock(27, 28);
  F.insertToBlock(29, 30);
  F.insertToBlock(31, 32);
  F.insertToBlock(25, 27);
  F.insertToBlock(29, 31);
  F.insertToBlock(25, 32);
  // F.statistic(false, false, true);

  F.insertToBlock(1, 9);
  F.insertToBlock(17, 25);
  F.insertToBlock(17, 1);

  F.insertToBlock(33, 34);
  F.insertToBlock(35, 36);
  F.insertToBlock(33, 35);
  F.insertToBlock(33, 1);

  F.insertToBlock(37, 38);
  F.insertToBlock(38, 1);
  F.insertToBlock(39, 33);

  F.insertToBlock(61, 62);
  F.insertToBlock(61, 63);
  // F.statistic(false, false, true);
  F.insertToBlock(63, 39);
  // F.statistic(false, false, true);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(5, 6);
  F.remove(7, 8);
  F.remove(1, 3);
  F.remove(5, 7);
  F.remove(1, 5);

  F.remove(9, 10);
  F.remove(11, 12);
  F.remove(9, 11);
  F.remove(13, 14);
  F.remove(15, 16);
  F.remove(13, 15);
  F.remove(9, 13);

  F.remove(17, 18);
  F.remove(19, 20);
  F.remove(21, 22);
  F.remove(23, 24);
  F.remove(21, 23);
  F.remove(17, 20);
  F.remove(17, 24);

  F.remove(25, 26);
  F.remove(27, 28);
  F.remove(29, 30);
  F.remove(31, 32);
  F.remove(25, 27);
  F.remove(29, 31);
  F.remove(25, 32);
  // F.statistic(false, false, true);

  F.remove(1, 9);
  F.remove(17, 25);
  F.remove(17, 1);

  F.remove(33, 34);
  F.remove(35, 36);
  F.remove(33, 35);
  F.remove(33, 1);

  F.remove(37, 38);
  F.remove(38, 1);
  F.remove(39, 33);

  F.remove(61, 62);
  F.remove(61, 63);
  // F.statistic(false, false, true);
  F.remove(63, 39);
}
void test15() {  // lp == lson
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(2, 6);
  F.insertToBlock(7, 2);
  F.insertToBlock(8, 4);
  F.insertToBlock(3, 1);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(5, 6);
  F.remove(2, 6);
  F.remove(2, 7);
  F.remove(8, 4);
  F.remove(3, 1);
  F.run_stat(".", true);
}
void test16() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(2, 4);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 6);
  F.insertToBlock(8, 2);
  F.insertToBlock(3, 6);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(2, 4);
  F.remove(5, 6);
  F.remove(7, 6);
  F.remove(8, 2);
  F.remove(3, 6);
  F.run_stat(".", true);
}
void test17() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(3, 1);
  F.insertToBlock(5, 2);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(3, 1);
  F.remove(5, 2);
  F.run_stat(".", true);
}
void test18() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 2);
  F.insertToBlock(4, 1);
  F.insertToBlock(5, 4);
  F.insertToBlock(6, 3);
  F.remove(1, 2);
  F.remove(3, 2);
  F.remove(4, 1);
  F.remove(5, 4);
  F.remove(6, 3);
  F.run_stat(".", true);
}
void test19() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(3, 1);
  F.insertToBlock(3, 5);
  F.insertToBlock(4, 2);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(3, 1);
  F.remove(3, 5);
  F.remove(4, 2);
  F.run_stat(".", true);
}
void test20() {
  SCCWN F(20);
  F.insertToBlock(4, 3);
  F.insertToBlock(2, 4);
  F.insertToBlock(1, 5);
  F.insertToBlock(3, 6);
  F.insertToBlock(2, 3);
  F.insertToBlock(5, 4);
  F.remove(4, 3);
  // F.remove(2, 4);
  F.run_stat(".", true);
}
void test21() {
  SCCWN F(10);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(4, 2);
  F.insertToBlock(1, 5);
  F.insertToBlock(5, 6);
  F.insertToBlock(7, 5);
  F.run_stat(".", true);
  F.remove(1, 2);
  F.run_stat(".", true);
  F.remove(3, 4);
  F.run_stat(".", true);
  F.remove(4, 2);
  F.remove(1, 5);
  F.remove(5, 6);
  F.remove(7, 5);
  F.run_stat(".", true);
}
void test22() {
  SCCWN F(20);
  F.insertToBlock(1, 2);
  F.insertToBlock(3, 4);
  F.insertToBlock(3, 5);
  F.insertToBlock(6, 1);
  F.insertToBlock(7, 2);
  F.insertToBlock(2, 8);
  F.insertToBlock(7, 8);
  F.insertToBlock(6, 5);
  F.run_stat(".", true);
  F.remove(1, 2);
  F.run_stat(".", true);
  F.remove(3, 4);
  F.run_stat(".", true);
  F.remove(3, 5);
  F.run_stat(".", true);
  F.remove(6, 1);
  F.run_stat(".", true);
  F.remove(7, 2);
  F.run_stat(".", true);
  F.remove(2, 8);
  F.run_stat(".", true);
  F.remove(7, 8);
  F.remove(6, 5);
  F.run_stat(".", true);
}
void test23() {
  // size_t n = 64;
  size_t n = 2048;
  SCCWN F(n);
  F.lmax = std::log2(n) + 1;
  parlay::sequence<std::pair<size_t, size_t>> e;
  for (size_t i = 1; i < n - 1; i++)
    for (size_t j = i + 1; j <= n - 1; j++)
      e.push_back(std::make_pair(i, j));

  // for (size_t i = 0; i < ins.size(); i++)
  //   std::cout << ins[i].first << " " << ins[i].second << std::endl;
  auto ins = e.cut(0, 1000000);
  // auto ins = e;
  for (size_t i = 0; i < ins.size(); i++)
    F.insertToBlock(ins[i].first, ins[i].second);
  // F.run_stat(".", true);
  auto del = ins.cut(0, 500000);
  auto db1 = parlay::random_shuffle(del);
  for (size_t i = 0; i < db1.size(); i++) {
    F.remove(db1[i].first, db1[i].second);
  }
  auto ib1 = parlay::random_shuffle(db1);
  for (size_t i = 0; i < ib1.size(); i++)
    F.insertToBlock(ib1[i].first, ib1[i].second);
  for (size_t i = 0; i < ins.size(); i++)
    F.remove(ins[i].first, ins[i].second);
}
// void test_local() {
//   size_t n = 2049280;
//   SCCWN F(n);
//   F.lmax = std::log2(n) + 1;

//   std::ifstream fin;
//   fin.open("ap4.in");
//   while (!fin.eof()) {
//     size_t x, y;
//     fin >> x >> y;
//     F.insertToBlock(x, y);
//   }
//   fin.close();

//   std::ifstream fdel;
//   fdel.open("ap4.in");
//   while (!fin.eof()) {
//     size_t x, y;
//     fdel >> x >> y;
//     F.remove(x, y);
//   }
//   fdel.close();
// }
int main() {
  // test1();
  // test2();
  // test3();
  // test4();
  // // test5();
  // test6();
  // test7();
  // test8();
  // test9();
  // test10();
  // test11();
  // test12();
  // test13();
  // test14();
  // test15();
  // test16();
  // test17();
  // test18();
  // test19();
  // test20();
  // test21();
  // test22();
  test23();
  // test_local();
  return 0;
}