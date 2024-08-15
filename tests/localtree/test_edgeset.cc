#include "parlay/internal/get_time.h"
#include "parlay/utilities.h"
#include <cstddef>
#include <dycon/localTree/edgeset.hpp>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <set>
void test() {
  DynamicArray<size_t> A;
  for (size_t i = 0; i < 66; i++)
    A.insert(i);
  A.print_all();
  std::cout << "----------\n";
  for (size_t i = 0; i < 22; i++)
    std::cout << A.pop() << std::endl;
  std::cout << "----------\n";
  A.print_all();
  size_t ith = parlay::hash64(4396) % A.get_size();
  std::cout << "After remove " << A.at(ith) << std::endl;
  A.remove(ith);
  A.print_all();
}
void test_remove() {
  DynamicArray<size_t> A;
  for (size_t i = 1; i < 11; i++)
    A.insert(i);
  A.remove(5);
  A.print_all();
}
//      insert     pop
// 8,   0.0039    0.0010
// 16,  0.0023    0.0006
// 32,  0.0022    0.0006
// 64,  0.0012    0.0002
// 128, 0.0011    0.0002
// 256, 0.0011    0.0002
// 512, 0.0012    0.0002
// 8,   0.0009    0.0004
// 16,  0.0003    0.0002
// 32   0.0002    0.0002
// set  0.0166    0.0055
void bench() {
  {
    DynamicArray<size_t, 64> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 64");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 64");
  }
  {
    DynamicArray<size_t, 8> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 8");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 8");
  }
  {
    DynamicArray<size_t, 16> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 16");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 16");
  }
  {
    DynamicArray<size_t, 32> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 32");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 32");
  }
  {
    DynamicArray<size_t, 64> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 64");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 64");
  }
  {
    DynamicArray<size_t, 128> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 128");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 128");
  }
  {
    DynamicArray<size_t, 256> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 256");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 256");
  }
  {
    DynamicArray<size_t, 512> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 512");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 512");
  }
  {
    DynamicArray<size_t, 8> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 8");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 8");
  }
  {
    DynamicArray<size_t, 16> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 16");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 16");
  }
  {
    DynamicArray<size_t, 32> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with block size 32");
    A.mem_stat();
    t.next("print_stat");
    for (size_t i = 0; i < 100000; i++)
      size_t e = A.pop();
    t.next("pop with block size 32");
  }
  {
    std::set<size_t> A;
    parlay::internal::timer t;
    for (size_t i = 0; i < 100000; i++)
      A.insert(i);
    t.next("insert with std::set");
    for (size_t i = 0; i < 100000; i++)
      A.erase(i);
    t.next("pop with std::set");
  }
}
int main() {

  // test();
  test_remove();
  // bench();
  return 0;
}
