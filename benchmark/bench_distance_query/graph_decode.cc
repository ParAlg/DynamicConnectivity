#include "data_generator.hpp"
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
  edges E;
  size_t n, m;
  // edges E(29541285);
  // size_t n = 6024271, m = 29541285;
  std::tie(E, n, m) = graph_decoder(In);
  utils::decode_edges(E, Out + ".txt");
  // size_t i = 0;
  // std::ifstream fin;
  // fin.open(In);
  // while (!fin.eof()) {
  //   fin >> E[i].first >> E[i].second;
  //   i++;
  // }
  // fin.close();
  size_t num_batches = atoi(argv[3]);
  size_t num_queries = atoi(argv[4]);
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

  // generate_dis_query(E, "query_pairs.txt", num_queries);

  return 0;
}