#include <dycon/localTree/CWN.hpp>
#include <fstream>
#include <iostream>
void test1() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(1, 3);
  F.insert(1, 4);
  F.insert(2, 3);
  F.insert(3, 4);
  F.insert(2, 4);
  F.remove(1, 2);
  F.remove(1, 3);
  F.remove(1, 4);
  F.remove(2, 4);
  F.run_stat("./", true);
}
void test2() { // not compression
  CWN F(20);
  F.insert(1, 2);
  F.insert(2, 3);
  F.insert(4, 5);
  F.insert(5, 3);
  F.insert(5, 6);
  F.insert(7, 5);
  F.remove(1, 2);
  F.remove(2, 3);
  F.remove(4, 5);
  F.remove(5, 3);
  F.remove(5, 6);
  F.remove(7, 5);
  F.run_stat("./", true);
}
void test3() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(1, 3);
  F.insert(5, 6);
  F.insert(1, 5);
  F.insert(1, 7);
  F.insert(3, 5);
  F.insert(1, 8);
  F.insert(7, 8);
  // F.remove(1, 8);
  // F.remove(3, 4);
  F.remove(1, 3);
  F.run_stat(".", true);
}
void test4() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(1, 3);
  F.remove(1, 2);
  F.remove(1, 3);
  F.run_stat(".", true);
}
void test5() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(7, 8);
  F.insert(1, 3);
  F.insert(5, 7);
  F.insert(4, 6);
  F.insert(9, 10);
  F.insert(9, 1);
  F.insert(10, 1);
  F.insert(11, 2);
  // F.statistic(false, false, true);
  F.insert(11, 10);
  F.insert(5, 11);
  F.insert(12, 2);
  // F.statistic(false, false, true);
}
void test6() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(1, 3);
  F.insert(5, 4);
  F.insert(4, 2);
  F.remove(1, 2);
  F.remove(1, 3);
  F.remove(4, 2);
  F.remove(5, 4);
  F.run_stat(".", true);
}
void test7() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(1, 3);
  F.insert(5, 6);
  F.insert(6, 7);
  F.insert(3, 5);
  F.insert(1, 8);
  F.insert(5, 8);
  // F.statistic(false, false, true);
}
void test8() {
  CWN F(50);

  F.insert(1, 2);
  F.insert(1, 3);
  F.insert(1, 4);
  F.insert(2, 3);
  F.insert(2, 4);
  F.insert(3, 4);

  F.insert(5, 6);
  F.insert(6, 7);
  F.insert(7, 8);

  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(10, 11);
  F.insert(13, 14);
  F.insert(15, 13);
  F.insert(16, 9);
  F.insert(13, 16);
  // F.statistic(false, false, true);
}
void test9() {
  CWN F(50);

  F.insert(1, 2);
  F.insert(1, 3);
  F.insert(1, 4);
  F.insert(2, 3);
  F.insert(2, 4);
  F.insert(3, 4);

  F.insert(5, 6);
  F.insert(6, 7);
  F.insert(7, 8);

  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(10, 11);
  F.insert(13, 14);
  F.insert(15, 13);
  F.insert(16, 9);
  F.insert(15, 16);
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
  CWN F(100);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(7, 8);
  F.insert(1, 3);
  F.insert(5, 7);
  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(13, 14);
  F.insert(15, 16);
  F.insert(9, 11);
  F.insert(13, 15);
  F.insert(9, 13);

  F.insert(17, 18);
  F.insert(19, 20);
  F.insert(21, 22);
  F.insert(23, 24);
  F.insert(17, 19);
  F.insert(21, 23);
  F.insert(17, 21);
  F.insert(9, 17);

  F.insert(1, 9);
  F.insert(9, 25);
  // F.statistic(false, false, true);
  F.insert(1, 25);
  // F.statistic(false, false, true);
}
void test11() {
  CWN F(100);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(7, 8);
  F.insert(1, 3);
  F.insert(5, 7);
  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(13, 14);
  F.insert(15, 16);
  F.insert(9, 11);
  F.insert(13, 15);
  F.insert(9, 13);

  F.insert(17, 18);
  F.insert(19, 20);
  F.insert(21, 22);
  F.insert(23, 24);
  F.insert(17, 19);
  F.insert(21, 23);
  F.insert(17, 21);
  F.insert(9, 17);

  F.insert(1, 9);

  // F.statistic(false, false, true);
  F.insert(1, 25);
  // F.statistic(false, false, true);
}
void test12() {
  CWN F(120);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(7, 8);
  F.insert(1, 3);
  F.insert(5, 7);
  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(9, 11);
  F.insert(13, 14);
  F.insert(15, 16);
  F.insert(13, 15);
  F.insert(9, 13);

  F.insert(17, 18);
  F.insert(19, 20);
  F.insert(21, 22);
  F.insert(23, 24);
  F.insert(21, 23);
  F.insert(17, 20);
  F.insert(17, 24);

  F.insert(25, 26);
  F.insert(27, 28);
  F.insert(29, 30);
  F.insert(31, 32);
  F.insert(25, 27);
  F.insert(29, 31);
  F.insert(25, 32);

  F.insert(1, 9);
  F.insert(17, 25);
  F.insert(17, 1);
  // F.statistic(false, false, true);

  F.insert(33, 34);
  F.insert(35, 36);
  F.insert(33, 35);
  F.insert(1, 33);
  // F.statistic(false, false, true);

  F.insert(37, 33);
  // F.statistic(false, false, true);
  F.insert(38, 33);
  // F.statistic(false, false, true);

  F.insert(39, 40);
  F.insert(39, 1);
  // F.statistic(false, false, true);

  F.insert(39, 33);
  // F.statistic(false, false, true);
}
void test13() {
  CWN F(120);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(7, 8);
  F.insert(1, 3);
  F.insert(5, 7);
  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(9, 11);
  F.insert(13, 14);
  F.insert(15, 16);
  F.insert(13, 15);
  F.insert(9, 13);

  F.insert(17, 18);
  F.insert(19, 20);
  F.insert(21, 22);
  F.insert(23, 24);
  F.insert(21, 23);
  F.insert(17, 20);
  F.insert(17, 24);

  F.insert(25, 26);
  F.insert(27, 28);
  F.insert(29, 30);
  F.insert(31, 32);
  F.insert(25, 27);
  F.insert(29, 31);
  F.insert(25, 32);
  // F.statistic(false, false, true);

  F.insert(1, 9);
  F.insert(17, 25);
  F.insert(17, 1);

  F.insert(33, 34);
  F.insert(35, 36);
  F.insert(33, 35);
  F.insert(1, 33);
  // F.statistic(false, false, true);

  F.insert(37, 33);
  F.insert(38, 33);
  // F.statistic(false, false, true);

  F.insert(39, 40);
  F.insert(39, 1);
  // F.statistic(false, false, true);

  F.insert(39, 38);
  // F.statistic(false, false, true);
}
void test14() {
  CWN F(120);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(7, 8);
  F.insert(1, 3);
  F.insert(5, 7);
  F.insert(1, 5);

  F.insert(9, 10);
  F.insert(11, 12);
  F.insert(9, 11);
  F.insert(13, 14);
  F.insert(15, 16);
  F.insert(13, 15);
  F.insert(9, 13);

  F.insert(17, 18);
  F.insert(19, 20);
  F.insert(21, 22);
  F.insert(23, 24);
  F.insert(21, 23);
  F.insert(17, 20);
  F.insert(17, 24);

  F.insert(25, 26);
  F.insert(27, 28);
  F.insert(29, 30);
  F.insert(31, 32);
  F.insert(25, 27);
  F.insert(29, 31);
  F.insert(25, 32);
  // F.statistic(false, false, true);

  F.insert(1, 9);
  F.insert(17, 25);
  F.insert(17, 1);

  F.insert(33, 34);
  F.insert(35, 36);
  F.insert(33, 35);
  F.insert(33, 1);

  F.insert(37, 38);
  F.insert(38, 1);
  F.insert(39, 33);

  F.insert(61, 62);
  F.insert(61, 63);
  // F.statistic(false, false, true);
  F.insert(63, 39);
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
void test15() { // lp == lson
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(5, 6);
  F.insert(2, 6);
  F.insert(7, 2);
  F.insert(8, 4);
  F.insert(3, 1);
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
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(2, 4);
  F.insert(5, 6);
  F.insert(7, 6);
  F.insert(8, 2);
  F.insert(3, 6);
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
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(3, 1);
  F.insert(5, 2);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(3, 1);
  F.remove(5, 2);
  F.run_stat(".", true);
}
void test18() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 2);
  F.insert(4, 1);
  F.insert(5, 4);
  F.insert(6, 3);
  F.remove(1, 2);
  F.remove(3, 2);
  F.remove(4, 1);
  F.remove(5, 4);
  F.remove(6, 3);
  F.run_stat(".", true);
}
void test19() {
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(3, 1);
  F.insert(3, 5);
  F.insert(4, 2);
  F.remove(1, 2);
  F.remove(3, 4);
  F.remove(3, 1);
  F.remove(3, 5);
  F.remove(4, 2);
  F.run_stat(".", true);
}
void test20() {
  CWN F(20);
  F.insert(4, 3);
  F.insert(2, 4);
  F.insert(1, 5);
  F.insert(3, 6);
  F.insert(2, 3);
  F.insert(5, 4);
  F.remove(4, 3);
  // F.remove(2, 4);
  F.run_stat(".", true);
}
void test21() {
  CWN F(10);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(4, 2);
  F.insert(1, 5);
  F.insert(5, 6);
  F.insert(7, 5);
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
  CWN F(20);
  F.insert(1, 2);
  F.insert(3, 4);
  F.insert(3, 5);
  F.insert(6, 1);
  F.insert(7, 2);
  F.insert(2, 8);
  F.insert(7, 8);
  F.insert(6, 5);
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
  CWN F(n);
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
// void test_local() {
//   size_t n = 2049280;
//   CWN F(n);
//   F.lmax = std::log2(n) + 1;

//   std::ifstream fin;
//   fin.open("ap4.in");
//   while (!fin.eof()) {
//     size_t x, y;
//     fin >> x >> y;
//     F.insert(x, y);
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