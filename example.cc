#include <algorithm>
#include <cstdint>
#include <dycon/localTree/SCCWN.hpp>
#include <utility>
#include <vector>
int main() {
  // set up example insertions and deletions
  uint32_t n = 20;
  uint32_t m = n - 1;
  std::vector<std::pair<uint32_t, uint32_t>> insertions(m);
  std::vector<std::pair<uint32_t, uint32_t>> deletions(m);
  for (auto i = 0; i < m; i++)
    insertions[i] = deletions[i] = std::pair(i, i + 1);
  std::random_shuffle(deletions.begin(), deletions.end());

  // prepare to use dynamic connectivity structure
  SCCWN F(n);

  // Insertion
  for (auto i = 0; i < m; i++) {
    // call to insert
    F.insert(insertions[i].first, insertions[i].second);
    // statistics
    std::cout << "Connected Component Statistics" << std::endl;
    auto CCs = F.CC_stat();
    std::cout << "number of CCs: " << CCs.size() << std::endl;
    for (auto j = 0; j < CCs.size(); j++) {
      std::cout << "size of CC: " << CCs[j].second << std::endl;
    }
  }

  // deletion
  for (auto i = 0; i < m; i++) {
    F.remove(deletions[i].first, deletions[i].second);
    // statistics
    std::cout << "Connected Component Statistics" << std::endl;
    auto CCs = F.CC_stat();
    std::cout << "number of CCs: " << CCs.size() << std::endl;
    for (auto j = 0; j < CCs.size(); j++) {
      std::cout << "size of CC: " << CCs[j].second << std::endl;
    }
  }
  return 0;
}