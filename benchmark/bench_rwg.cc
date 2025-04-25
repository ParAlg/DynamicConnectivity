#include "bench_all.hpp"
int main(int argc, char **argv) {

  std::string usage = "Usage: Connectivity [-a #algorithms] [-b #batches] [-q "
                      "#queries/batch] <input_file> <output_file>";
  commandLine P(argc, argv, usage);
  auto IOF = P.IOFileNames();
  std::string In = IOF.first;
  std::string Out = IOF.second;
  uint32_t algorithms = P.getOptionIntValue("-a", 0);
  uint32_t num_testpoints = P.getOptionIntValue("-b", 10);
  uint32_t num_queries = P.getOptionIntValue("-q", 1000);

  auto G = utils::break_sym_graph_from_bin(In);
  vertex n = G.size();

  auto E = parlay::remove_duplicates_ordered(utils::to_edges(G),
                                             [&](edge a, edge b) {
                                               if (a.first == b.first)
                                                 return a.second < b.second;
                                               return a.first < b.first;
                                             });
  vertex m = E.size();
  std::cout << n << std::endl << m << std::endl;
  E = parlay::random_shuffle(E);

  auto batch_size = parlay::tabulate(
      num_testpoints + 1, [&](uint32_t i) { return m / num_testpoints * i; });
  batch_size[num_testpoints] = m;

  auto batches_ins = parlay::tabulate(num_testpoints, [&](uint32_t i) {
    return parlay::to_sequence(E.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_ins =
      utils::generate_CC_queries(num_testpoints, batches_ins, n, num_queries);

  auto E_ = parlay::random_shuffle(E);
  auto batches_del = parlay::tabulate(num_testpoints, [&](uint32_t i) {
    return parlay::to_sequence(E_.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_del =
      utils::generate_CC_queries(num_testpoints, batches_del, n, num_queries);
#ifndef MEM_USE
  parlay::execute_with_scheduler(1, [&]() {
#endif
    switch (algorithms) {
    case 0:
      bench_seq_hdt(num_testpoints, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      bench_compress_CWN_root(num_testpoints, n,
                              Out + "_compressed_CWN_root.ans", batches_ins,
                              queries_ins, batches_del, queries_del);
      bench_compress_CWN_lca(num_testpoints, n, Out + "_compressed_CWN_lca.ans",
                             batches_ins, queries_ins, batches_del,
                             queries_del);
      break;
    case 1:
      bench_seq_hdt(num_testpoints, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      break;
    case 2:
      bench_compress_CWN_root(num_testpoints, n,
                              Out + "_compressed_CWN_root.ans", batches_ins,
                              queries_ins, batches_del, queries_del);
      break;
    case 3:
      bench_compress_CWN_lca(num_testpoints, n, Out + "_compressed_CWN_lca.ans",
                             batches_ins, queries_ins, batches_del,
                             queries_del);
      break;
    case 4:
      bench_compress_DyCWN_root(num_testpoints, n,
                                Out + "_compressed_DyCWN_root.ans", batches_ins,
                                queries_ins, batches_del, queries_del);
      break;
    case 5:
      bench_compress_DyCWN_lca(num_testpoints, n,
                               Out + "_compressed_DyCWN_lca.ans", batches_ins,
                               queries_ins, batches_del, queries_del);
      break;
    case 6:
      bench_seq_hdt(num_testpoints, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      bench_compress_DyCWN_root(num_testpoints, n,
                                Out + "_compressed_DyCWN_root.ans", batches_ins,
                                queries_ins, batches_del, queries_del);
      bench_compress_DyCWN_lca(num_testpoints, n,
                               Out + "_compressed_DyCWN_lca.ans", batches_ins,
                               queries_ins, batches_del, queries_del);
      break;
    }
#ifndef MEM_USE
  });
#endif
  return 0;
}