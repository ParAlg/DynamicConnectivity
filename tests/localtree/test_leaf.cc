#include "parlay/internal/group_by.h"
#include <cstddef>
#include <dycon/helpers/assert.hpp>
#include <dycon/localTree/leaf.hpp>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
void test_lazy() {
  using dataType =
      parlay::sequence<std::pair<std::pair<size_t, size_t>, size_t>>;
  size_t n = 500;
  size_t m = n * n;
  size_t logn = parlay::log2_up(n);
  dataType Updates(m);
  parlay::parallel_for(0, n, [&](size_t i) {
    parlay::parallel_for(0, n, [&](size_t j) {
      Updates[i * n + j].first.first = i;
      Updates[i * n + j].first.second = j;
      Updates[i * n + j].second = parlay::hash64(i * j) % logn;
    });
  });
  Updates = parlay::random_shuffle(Updates);
  Updates = parlay::remove_duplicates(Updates);
  leaf *CL = new leaf[m];
  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    CL[T.first.first].insert(T.first.second, T.second);
    ASSERT_MSG(CL[T.first.first].getEdgeMap()[T.second] == true,
               "set bitmap fail");
  }

  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    ASSERT_MSG(CL[T.first.first].getEdgeInfo(T.first.second).first == T.second,
               "get wrong level");
  }

  for (size_t i = m - 1; i >= 0; i--) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    CL[T.first.first].remove_lazy(T.first.second, T.second);
    if (!i)
      break;
  }
  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    assert(CL[T.first.first].hasLevelEdge(T.second) == false);
    assert(CL[T.first.first].getEdgeMap()[T.second] == false);
  }
  delete[] CL;
}
void test_remove() {
  leaf *lf = new leaf;
  for (size_t i = 1; i < 11; i++)
    lf->insert(i, 1);
  lf->flattenLevel(1);
  std::cout << "after remove 5th\n";
  lf->remove(5, 5, 1);
  lf->flattenLevel(1);
  std::cout << "after remove 2nd\n";
  lf->remove(2, 2, 1);
  lf->flattenLevel(1);
  delete lf;
}
int main() {
  // test_lazy();
  test_remove();
  return 0;
}