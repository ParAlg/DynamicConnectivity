#include "rankTree.hpp"
#include "assert.hpp"
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <iostream>
void test_basic(size_t n = 10) {}
void abandon1() {
  //  { auto test_decomposetion = [](rankTree *t) {
  //     std::cout << "test decomposition" << std::endl;
  //     auto ans = rankTree::decompose(rankTree::getRoot(t));
  //     for (size_t i = 0; i < ans.size(); i++)
  //       std::cout << ans[i]->getRank() << std::endl;
  //   };

  //   auto test_remove = [&](rankTree *t) {
  //     std::cout << "before remove" << std::endl;
  //     auto ans = rankTree::decompose(t);
  //     for (size_t i = 0; i < ans.size(); i++)
  //       std::cout << ans[i]->getRank() << std::endl;
  //     std::cout << "after remove " << parlay::hash64(ans[0]->getRank()) % ans.size() << "th with rank "
  //               << ans[parlay::hash64(ans[0]->getRank()) % ans.size()]->getRank() << std::endl;
  //     ans = rankTree::remove(ans[parlay::hash64(ans[0]->getRank()) % ans.size()]);

  //     for (size_t i = 0; i < ans.size(); i++)
  //       std::cout << ans[i]->getRank() << std::endl;
  //     std::cout << std::endl << std::endl;
  //   };
  //   for (auto it : ans2)
  //     test_remove(it);}
}
int main() {
  size_t n = 50000;
  std::vector<rankTree *> A(n);
  parlay::parallel_for(0, n, [&](size_t i) { A[i] = new rankTree(parlay::hash64(i) % parlay::log2_up(n)); });
  auto comp = [&](const rankTree *T1, const rankTree *T2) { return T1->getRank() < T2->getRank(); };
  parlay::sort_inplace(A, comp);
  for (size_t i = 0; i < n - 1; i++)
    ASSERT_MSG(A[i]->getRank() <= A[i + 1]->getRank(), "sort rank fail");
  std::cout << "test rank tree generation" << std::endl;
  // for (size_t i = 0; i < n; i++)
  //   std::cout << A[i]->getRank() << std::endl;
  std::cout << "test building rank trees with distinct rank from a bunch of rank trees" << std::endl;
  auto ans1 = rankTree::buildFromSequence(A);

  // for (size_t i = 0; i < ans1.size(); i++)
  //   std::cout << ans1[i]->getRank() << std::endl;
  for (size_t i = 0; i < ans1.size() - 1; i++)
    ASSERT_MSG(ans1[i]->getRank() < ans1[i + 1]->getRank(), "not distinct rank trees");

  std::vector<rankTree *> B(n);
  parlay::parallel_for(0, n, [&](size_t i) { B[i] = new rankTree(parlay::hash64_2(i) % parlay::log2_up(n)); });
  parlay::sort_inplace(B, comp);
  for (size_t i = 0; i < n - 1; i++)
    ASSERT_MSG(B[i]->getRank() <= B[i + 1]->getRank(), "sort rank fail");
  std::cout << "test rank tree generation" << std::endl;
  // for (size_t i = 0; i < n; i++)
  //   std::cout << B[i]->getRank() << std::endl;
  std::cout << "test building rank trees with distinct rank from a bunch of rank trees" << std::endl;
  auto ans2 = rankTree::buildFromSequence(B);
  // for (size_t i = 0; i < ans2.size(); i++)
  //   std::cout << ans2[i]->getRank() << std::endl;
  for (size_t i = 0; i < ans2.size() - 1; i++)
    ASSERT_MSG(ans2[i]->getRank() < ans2[i + 1]->getRank(), "not distinct rank trees");

  std::cout << "test merging two sequences of rank trees" << std::endl;
  auto ans3 = rankTree::Merge(ans1, ans2, nullptr);
  for (size_t i = 0; i < ans3.size(); i++)
    std::cout << ans3[i]->getRank() << std::endl;
  for (size_t i = 0; i < ans3.size() - 1; i++)
    ASSERT_MSG(ans3[i]->getRank() < ans3[i + 1]->getRank(), "not distinct rank trees");

  for (auto it : ans3) {
    auto splay = rankTree::decompose(it);
    std::vector<size_t> counts(parlay::log2_up(n) + 1);
    counts.assign(parlay::log2_up(n), 0);
    for (auto i : splay)
      counts[i->getRank()]++;
    size_t p = parlay::hash64(splay.size()) % splay.size();
    counts[splay[p]->getRank()]--;
    std::vector<size_t> match(parlay::log2_up(n) + 1);
    match.assign(parlay::log2_up(n), 0);
    auto removed = rankTree::remove(splay[p], nullptr);
    for (auto i : removed) {
      auto q = rankTree::decompose(i);
      for (auto j : q)
        match[j->getRank()]++;
    }
    parlay::parallel_for(0, counts.size(), [&](size_t i) { ASSERT_MSG(counts[i] == match[i], "remove fail"); });
  }
  return 0;
}