// #include "blocked_arr.hpp"
// #include <iostream>
// #include <fstream>
// void test1() {
//   blocked_arr F(20);
//   F.insert(1, 2);
//   F.insert(1, 3);
//   F.statistic(false, false, true);
// }
// void test2() {
//   blocked_arr F(20);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   // F.statistic(false, false, true);
//   F.insert(1, 3);
//   // F.statistic(false, false, true);
//   F.insert(1, 4);
//   // F.statistic(false, false, true);
//   // F.insert(2, 3);
//   // F.insert(2, 4);
//   // F.statistic(false, false, true);
// }
// void test3() {
//   blocked_arr F(20);
//   F.insert(1, 2);
//   F.insert(1, 4);
//   F.insert(1, 3);
//   F.statistic(false, false, true);
// }
// void test4() {
//   blocked_arr F(20);
//   F.insert(1, 2);
//   F.insert(1, 4);
//   F.insert(1, 3);
//   F.statistic(false, false, true);
//   F.insert(3, 4);
//   F.statistic(false, false, true);
// }
// void test5() {
//   blocked_arr F(20);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(5, 6);
//   F.insert(7, 8);
//   F.insert(1, 3);
//   F.insert(5, 7);
//   F.insert(4, 6);
//   F.insert(9, 10);
//   F.insert(9, 1);
//   F.insert(10, 1);
//   F.insert(11, 2);
//   F.statistic(false, false, true);
//   F.insert(11, 10);
//   F.insert(5, 11);
//   F.insert(12, 2);
//   F.statistic(false, false, true);
// }
// void test6() {
//   // blocked_arr F(2049280);
//   // F.insert(1488997, 1525594);
//   // F.insert(1525594, 1970860);
//   // F.insert(1367482, 1525594);
//   // F.insert(1525594, 1666484);
//   // F.insert(812774, 1666484);
//   // F.insert(850407, 1666484);
//   // F.insert(1408804, 1970860);
//   // F.insert(850407, 1392420);
//   // F.insert(1392420, 1408804);
//   // F.statistic(false, false, true);
// }
// void test7() {
//   blocked_arr F(20);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(1, 3);
//   F.insert(5, 6);
//   F.insert(6, 7);
//   F.insert(3, 5);
//   F.insert(1, 8);
//   F.insert(5, 8);
//   F.statistic(false, false, true);
// }
// void test8() {
//   blocked_arr F(50);

//   F.insert(1, 2);
//   F.insert(1, 3);
//   F.insert(1, 4);
//   F.insert(2, 3);
//   F.insert(2, 4);
//   F.insert(3, 4);

//   F.insert(5, 6);
//   F.insert(6, 7);
//   F.insert(7, 8);

//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(10, 11);
//   F.insert(13, 14);
//   F.insert(15, 13);
//   F.insert(16, 9);
//   F.insert(13, 16);
//   F.statistic(false, false, true);
// }
// void test9() {
//   blocked_arr F(50);

//   F.insert(1, 2);
//   F.insert(1, 3);
//   F.insert(1, 4);
//   F.insert(2, 3);
//   F.insert(2, 4);
//   F.insert(3, 4);

//   F.insert(5, 6);
//   F.insert(6, 7);
//   F.insert(7, 8);

//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(10, 11);
//   F.insert(13, 14);
//   F.insert(15, 13);
//   F.insert(16, 9);
//   F.insert(15, 16);
//   F.statistic(false, false, true);
// }
// void test10() {
//   blocked_arr F(100);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(5, 6);
//   F.insert(7, 8);
//   F.insert(1, 3);
//   F.insert(5, 7);
//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(13, 14);
//   F.insert(15, 16);
//   F.insert(9, 11);
//   F.insert(13, 15);
//   F.insert(9, 13);

//   F.insert(17, 18);
//   F.insert(19, 20);
//   F.insert(21, 22);
//   F.insert(23, 24);
//   F.insert(17, 19);
//   F.insert(21, 23);
//   F.insert(17, 21);
//   F.insert(9, 17);

//   F.insert(1, 9);
//   F.insert(9, 25);
//   // F.statistic(false, false, true);
//   F.insert(1, 25);
//   F.statistic(false, false, true);
// }
// void test11() {
//   blocked_arr F(100);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(5, 6);
//   F.insert(7, 8);
//   F.insert(1, 3);
//   F.insert(5, 7);
//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(13, 14);
//   F.insert(15, 16);
//   F.insert(9, 11);
//   F.insert(13, 15);
//   F.insert(9, 13);

//   F.insert(17, 18);
//   F.insert(19, 20);
//   F.insert(21, 22);
//   F.insert(23, 24);
//   F.insert(17, 19);
//   F.insert(21, 23);
//   F.insert(17, 21);
//   F.insert(9, 17);

//   F.insert(1, 9);

//   // F.statistic(false, false, true);
//   F.insert(1, 25);
//   F.statistic(false, false, true);
// }
// void test12() {
//   blocked_arr F(120);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(5, 6);
//   F.insert(7, 8);
//   F.insert(1, 3);
//   F.insert(5, 7);
//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(9, 11);
//   F.insert(13, 14);
//   F.insert(15, 16);
//   F.insert(13, 15);
//   F.insert(9, 13);

//   F.insert(17, 18);
//   F.insert(19, 20);
//   F.insert(21, 22);
//   F.insert(23, 24);
//   F.insert(21, 23);
//   F.insert(17, 20);
//   F.insert(17, 24);

//   F.insert(25, 26);
//   F.insert(27, 28);
//   F.insert(29, 30);
//   F.insert(31, 32);
//   F.insert(25, 27);
//   F.insert(29, 31);
//   F.insert(25, 32);

//   F.insert(1, 9);
//   F.insert(17, 25);
//   F.insert(17, 1);
//   // F.statistic(false, false, true);

//   F.insert(33, 34);
//   F.insert(35, 36);
//   F.insert(33, 35);
//   F.insert(1, 33);
//   // F.statistic(false, false, true);

//   F.insert(37, 33);
//   // F.statistic(false, false, true);
//   F.insert(38, 33);
//   // F.statistic(false, false, true);

//   F.insert(39, 40);
//   F.insert(39, 1);
//   // F.statistic(false, false, true);

//   F.insert(39, 33);
//   F.statistic(false, false, true);
// }
// void test13() {
//   blocked_arr F(120);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(5, 6);
//   F.insert(7, 8);
//   F.insert(1, 3);
//   F.insert(5, 7);
//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(9, 11);
//   F.insert(13, 14);
//   F.insert(15, 16);
//   F.insert(13, 15);
//   F.insert(9, 13);

//   F.insert(17, 18);
//   F.insert(19, 20);
//   F.insert(21, 22);
//   F.insert(23, 24);
//   F.insert(21, 23);
//   F.insert(17, 20);
//   F.insert(17, 24);

//   F.insert(25, 26);
//   F.insert(27, 28);
//   F.insert(29, 30);
//   F.insert(31, 32);
//   F.insert(25, 27);
//   F.insert(29, 31);
//   F.insert(25, 32);
//   // F.statistic(false, false, true);

//   F.insert(1, 9);
//   F.insert(17, 25);
//   F.insert(17, 1);

//   F.insert(33, 34);
//   F.insert(35, 36);
//   F.insert(33, 35);
//   F.insert(1, 33);
//   // F.statistic(false, false, true);

//   F.insert(37, 33);
//   F.insert(38, 33);
//   // F.statistic(false, false, true);

//   F.insert(39, 40);
//   F.insert(39, 1);
//   // F.statistic(false, false, true);

//   F.insert(39, 38);
//   F.statistic(false, false, true);
// }
// void test14() {
//   blocked_arr F(120);
//   F.insert(1, 2);
//   F.insert(3, 4);
//   F.insert(5, 6);
//   F.insert(7, 8);
//   F.insert(1, 3);
//   F.insert(5, 7);
//   F.insert(1, 5);

//   F.insert(9, 10);
//   F.insert(11, 12);
//   F.insert(9, 11);
//   F.insert(13, 14);
//   F.insert(15, 16);
//   F.insert(13, 15);
//   F.insert(9, 13);

//   F.insert(17, 18);
//   F.insert(19, 20);
//   F.insert(21, 22);
//   F.insert(23, 24);
//   F.insert(21, 23);
//   F.insert(17, 20);
//   F.insert(17, 24);

//   F.insert(25, 26);
//   F.insert(27, 28);
//   F.insert(29, 30);
//   F.insert(31, 32);
//   F.insert(25, 27);
//   F.insert(29, 31);
//   F.insert(25, 32);
//   // F.statistic(false, false, true);

//   F.insert(1, 9);
//   F.insert(17, 25);
//   F.insert(17, 1);

//   F.insert(33, 34);
//   F.insert(35, 36);
//   F.insert(33, 35);
//   F.insert(33, 1);

//   F.insert(37, 38);
//   F.insert(38, 1);
//   F.insert(39, 33);

//   F.insert(61, 62);
//   F.insert(61, 63);
//   F.statistic(false, false, true);
//   F.insert(63, 39);
//   F.statistic(false, false, true);
// }
// void test_local() {
//   size_t n = 2049280;
//   blocked_arr F(n);
//   std::ifstream fin;
//   fin.open("ap3.txt");
//   while (!fin.eof()) {
//     size_t x, y;
//     fin >> x >> y;
//     F.insert(x, y);
//   }
//   fin.close();
// }
int main() {
  //   // test1();
  //   // test2();
  //   // test3();
  //   // test4();
  //   // test5();
  //   // test6();
  //   // test7();
  //   // test8();
  //   // test9();
  //   // test10();
  //   // test11();
  //   // test12();
  //   // test13();
  //   // test14();
  //   test_local();
  return 0;
}