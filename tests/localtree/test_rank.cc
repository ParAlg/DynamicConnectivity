#include <dycon/helpers/assert.hpp>
#include <dycon/localTree/rankTree.hpp>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
class test_rankTree : public rankTree {
public:
  // using arr = parlay::sequence<test_rankTree *>;
  using arr = parlay::sequence<rankTree *>;
  // static arr convertFromBase(arr &A);
  // static arr convertToBase(arr &A);
  static arr testBuildFromSequence(arr &_A);
  static arr testRankTreesGen(size_t n);
  static arr testDecompose(arr &A);
  static arr testDecompose(rankTree *T);
  static bool testEqualRanks(arr &A, arr &B);
  static arr testRemove(rankTree *T);
  static arr testMerge(arr &A, arr &B);
  static arr appendSort(arr &A, arr &B);
  static rankTree *getRoot(rankTree *T) { return rankTree::testGetRoot(T); }
  static arr testRemove(arr &A, rankTree *T);
};
// using arr = test_rankTree::arr;
using arr = test_rankTree::arr;
// arr test_rankTree::convertToBase(arr &A) {
//   arr B;
//   B.reserve(A.size());
//   parlay::parallel_for(0, A.size(), [&](size_t i) { B[i] =
//   reinterpret_cast<rankTree *>(A[i]); }); return B;
// }
// arr test_rankTree::convertFromBase(arr &A) {
//   arr B;
//   B.reserve(A.size());
//   parlay::parallel_for(0, A.size(), [&](size_t i) { B[i] =
//   reinterpret_cast<test_rankTree *>(A[i]); }); return B;
// }
arr test_rankTree::testRankTreesGen(size_t n) {
  return rankTree::testRankTreesGen(n);
}
arr test_rankTree::testBuildFromSequence(arr &_A) {
  auto A = _A;
  return rankTree::testBuildFromSequence(A);
}
arr test_rankTree::testDecompose(rankTree *T) {
  return rankTree::testDecompose(T);
}
arr test_rankTree::testDecompose(arr &A) {
  arr B;
  for (size_t i = 0; i < A.size(); i++) {
    auto C = rankTree::testDecompose(A[i]);
    rankTree::testRankInc(C, "decompose not increasing");
    B.append(C);
  }
  return testSortByRank(B);
}
bool test_rankTree::testEqualRanks(arr &A, arr &B) {
  return rankTree::testEqualRanks(A, B);
}
arr test_rankTree::testRemove(rankTree *T) { return rankTree::testRemove(T); }
arr test_rankTree::testMerge(arr &A, arr &B) {
  return rankTree::testMerge(A, B);
}
arr test_rankTree::appendSort(arr &A, arr &B) {
  auto C = A;
  C.append(B);
  return rankTree::testSortByRank(C);
}
arr test_rankTree::testRemove(arr &A, rankTree *T) {
  return rankTree::testRemove(A, T);
}
int main() {
  size_t n = 100000;
  size_t m = 50000;

  auto A = test_rankTree::testRankTreesGen(n);
  auto ATrees = test_rankTree::testBuildFromSequence(A);
  auto AFlatten = test_rankTree::testDecompose(ATrees);
  ASSERT_MSG(test_rankTree::testEqualRanks(A, AFlatten) == true,
             "decompose fail");

  auto B = test_rankTree::testRankTreesGen(m);
  auto BTrees = test_rankTree::testBuildFromSequence(B);
  auto BFlatten = test_rankTree::testDecompose(BTrees);
  ASSERT_MSG(test_rankTree::testEqualRanks(B, BFlatten) == true,
             "decompose fail");

  auto CTrees = test_rankTree::testMerge(ATrees, BTrees);
  auto CFlatten = test_rankTree::testDecompose(CTrees);
  auto C = test_rankTree::appendSort(A, B);
  ASSERT_MSG(test_rankTree::testEqualRanks(C, CFlatten) == true, "merge fail");

  for (size_t i = 0; i < CTrees.size(); i++) {
    auto root = CTrees[i];
    auto rFlatten = test_rankTree::testDecompose(test_rankTree::getRoot(root));
    auto p = parlay::hash64(4396) % rFlatten.size();
    auto it = rFlatten.begin() + p;
    auto DTrees = test_rankTree::testRemove(*it);
    rFlatten.erase(it);
    auto DFlatten = test_rankTree::testDecompose(DTrees);
    ASSERT_MSG(test_rankTree::testEqualRanks(rFlatten, DFlatten) == true,
               "remove decompose fail");
  }

  auto E = test_rankTree::testRankTreesGen(n + m);
  for (size_t i = 0; i < 50; i++) {
    auto ETrees = test_rankTree::testBuildFromSequence(E);
    auto it = E.begin() + parlay::hash64(i) % E.size();
    ETrees = test_rankTree::testRemove(ETrees, *it);
    E.erase(it);
    auto EFlatten = test_rankTree::testDecompose(ETrees);
    ASSERT_MSG(test_rankTree::testEqualRanks(EFlatten, E) == true,
               "remove from rank forest fail");
  }
  return 0;
}