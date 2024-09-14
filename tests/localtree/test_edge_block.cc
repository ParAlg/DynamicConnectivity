#include "dycon/localTree/alloc.h"
#include "dycon/localTree/graph.hpp"
#include "parlay/alloc.h"
#include "parlay/internal/get_time.h"
#include "parlay/parallel.h"
#include <array>
#include <cstdint>
#include <dycon/localTree/edgeset.hpp>
void test1() {
  using edge_set = DynamicArray<uint32_t, 64>;
  edge_set A;
  A.EBallocator = new type_allocator<std::array<uint32_t, 64>>(1);
  A.EBallocator->stats();
  parlay::internal::timer t;
  for (uint32_t i = 0; i < 200000000; i++)
    A.insert(i);
  t.next("insertion");
  A.EBallocator->stats();
}
void test_512() {
  using edge_set = DynamicArray<uint32_t, 64>;
  edge_set A;
  A.EBallocator = new type_allocator<std::array<uint32_t, 64>>(128);
  A.EBallocator->stats();
  parlay::internal::timer t;
  for (uint32_t i = 0; i < 200000000; i++)
    A.insert(i);
  t.next("insertion");
  A.EBallocator->stats();
}
void test_parlay_alloc() {
  using edge_set =
      DynamicArray<uint32_t, 64,
                   parlay::type_allocator<std::array<uint32_t, 64>>>;
  edge_set A;
  A.EBallocator = new parlay::type_allocator<std::array<uint32_t, 64>>;
  A.EBallocator->print_stats();
  parlay::internal::timer t;
  for (uint32_t i = 0; i < 200000000; i++)
    A.insert(i);
  t.next("insertion");
  A.EBallocator->print_stats();
}
int main() {
  //   test1();
  //   parlay::execute_with_scheduler(1, []() { test_parlay_alloc(); });
  test_parlay_alloc();
  //   test_512();
  return 0;
}