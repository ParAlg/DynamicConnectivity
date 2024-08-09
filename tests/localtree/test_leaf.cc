#include <dycon/helpers/assert.hpp>
#include <dycon/localTree/leaf.hpp>
#include <parlay/primitives.h>
#include <parlay/sequence.h>

int main() {
  using dataType =
      parlay::sequence<std::pair<std::pair<size_t, size_t>, size_t>>;
  size_t n = 5000;
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
  parlay::random_shuffle(Updates);
  leaf *CL = new leaf[m];
  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    CL[T.first.first].insert(T.first.second, T.second);
    ASSERT_MSG(CL[T.first.first].getEdgeMap()[T.second] == true,
               "set bitmap fail");
  }
  parlay::random_shuffle(Updates);
  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    ASSERT_MSG(CL[T.first.first].getLevel(T.first.second) == T.second,
               "get wrong level");
  }
  parlay::random_shuffle(Updates);
  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    CL[T.first.first].remove(T.first.second, T.second);
  }
  parlay::random_shuffle(Updates);
  for (size_t i = 0; i < m; i++) {
    std::pair<std::pair<size_t, size_t>, size_t> &T = Updates[i];
    ASSERT_MSG(CL[T.first.first].checkLevel(T.second) == false,
               "remove failuer, edge still exists.");
    ASSERT_MSG(CL[T.first.first].getEdgeMap()[T.second] == false,
               "set bitmap fail");
  }
  delete[] CL;
  return 0;
}