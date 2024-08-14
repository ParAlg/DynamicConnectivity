#include "data_generator.hpp"
#include "dycon/helpers/union_find.hpp"
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/random.h"
#include "parlay/sequence.h"
#include <cstddef>
#include <cstdlib>
#include <string>
int main(int argc, char **argv) {
  if (argc != 5) {
    std::cout << "usage:\n";
    exit(0);
  }

  std::string In = argv[1];
  std::string Out = argv[2];
  size_t num_batches = atoi(argv[3]);
  size_t num_queries = atoi(argv[4]);

  auto G = utils::break_sym_graph_from_bin(In);
  vertex n = G.size();
  auto E = parlay::remove_duplicates_ordered(utils::to_edges(G),
                                             [&](edge a, edge b) {
                                               if (a.first == b.first)
                                                 return a.second < b.second;
                                               return a.first < b.first;
                                             });
  utils::decode_edges(E, Out + ".txt");
  vertex m = E.size();
  std::cout << n << std::endl << m << std::endl;
  E = parlay::random_shuffle(E);

  // edgeset cut point
  auto batch_size = parlay::tabulate(
      num_batches + 1, [&](size_t i) { return m / num_batches * i; });
  batch_size[num_batches] = m;

  auto batches_ins = parlay::tabulate(num_batches, [&](size_t i) {
    return parlay::to_sequence(E.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_ins =
      utils::generate_CC_queries(num_batches, batches_ins, n, num_queries);

  auto E_ = parlay::random_shuffle(E);
  auto batches_del = parlay::tabulate(num_batches, [&](size_t i) {
    return parlay::to_sequence(E_.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_del =
      utils::generate_CC_queries(num_batches, batches_del, n, num_queries);

  parlay::parallel_for(0, num_batches, [&](size_t i) {
    printpair(batches_ins[i], Out + "_batch_ins_" + std::to_string(i) + ".txt");
    printpair(queries_ins[i], Out + "_query_ins_" + std::to_string(i) + ".txt");
    printpair(batches_del[i], Out + "_batch_del_" + std::to_string(i) + ".txt");
    printpair(queries_del[i], Out + "_query_del_" + std::to_string(i) + ".txt");
  });
  return 0;
}