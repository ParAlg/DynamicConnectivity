#include "SCCWN.hpp"
#include <dycon/helpers/graph_utils.hpp>
#include <dycon/helpers/parse_command_line.hpp>
#include <parlay/internal/get_time.h>

using vertex = size_t;
using utils = graph_utils<vertex>;
using edge = utils::edge;

using query = utils::query;
using queries = utils::queries;
using Pair = std::pair<size_t, size_t>;
using Ans = parlay::sequence<parlay::sequence<bool>>;

int main(int argc, char** argv) {

  std::string usage = "Usage: Connectivity [-b #batches] [-q #queries/batch] <input_file> <output_file>";
  commandLine P(argc, argv, usage);
  auto IOF = P.IOFileNames();
  std::string In = IOF.first;
  std::string Out = IOF.second;
  size_t num_batches = P.getOptionIntValue("-b", 10);
  size_t num_queries = P.getOptionIntValue("-q", 1000);

  std::cout << "INITIALIZING INPUT..." << std::endl;

  auto G = utils::break_sym_graph_from_bin(In);
  vertex n = G.size();

  auto E = parlay::random_shuffle(parlay::remove_duplicates(utils::to_edges(G)));
  vertex m = E.size();

  auto batch_size = parlay::tabulate(num_batches + 1, [&](size_t i) { return m / num_batches * i; });
  batch_size[num_batches] = m;

  auto batches_ins = parlay::tabulate(
      num_batches, [&](size_t i) { return parlay::to_sequence(E.cut(batch_size[i], batch_size[i + 1])); });
  auto queries_ins = utils::generate_CC_queries(num_batches, batches_ins, n, num_queries);
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) { Ans_ins[i].resize(batches_ins[i].size()); });

  auto E_ = parlay::random_shuffle(E);
  auto batches_del = parlay::tabulate(
      num_batches, [&](size_t i) { return parlay::to_sequence(E_.cut(batch_size[i], batch_size[i + 1])); });
  auto queries_del = utils::generate_CC_queries(num_batches, batches_del, n, num_queries);
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) { Ans_del[i].resize(batches_del[i].size()); });
  parlay::internal::timer t;
  std::ofstream fins, fdel;
  t.start();
  SCCWN F(n);
  F.lmax = std::ceil(std::log2(n));
  F.verbose = false;
  assert(F.lmax < 64);
  if (!F.verbose) t.next("initialization");

  // DO INSERTIONS
  for (size_t i = 0; i < num_batches; i++) {
    std::cout << "INSERTING BATCH " << i << std::endl;
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insert(u, v);
      // std::cout << u << " " << v << std::endl;
      // todo here: add edges to graph
    }
    if (!F.verbose) t.next("Insert batch #" + std::to_string(i));
    // for (size_t j = 0; j < queries_ins[i].size(); j++) {
    //   // todo here
    //   Ans_ins[i][j] = F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    // }
    // if (!F.verbose) t.next("Answer queries #" + std::to_string(i));
    F.print_cg_sizes();
    // F.run_stat("./", true, false, false);
  }

  // DO DELETIONS
  for (size_t i = 0; i < num_batches; i++) {
    std::cout << "DELETING BATCH " << i << std::endl;
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    if (!F.verbose) t.next("Delete batch #" + std::to_string(i));
    // for (size_t j = 0; j < queries_del[i].size(); j++) {
    //   // todo here
    //   Ans_del[i][j] = F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    // }
    // if (!F.verbose) t.next("Answer queries #" + std::to_string(i));
    F.print_cg_sizes();
    // F.run_stat("./", true, false, false);
  }


  // DO QUERIES
  // auto x = Out.find_first_of(".");
  // auto s = Out.substr(0, x);
  // // F.run_stat(s);
  // std::ofstream faq;
  // faq.open(Out);
  // if (!faq.is_open()) {
  //   std::cout << "cannot open output file\n";
  //   std::abort();
  // }
  // for (size_t i = 0; i < num_batches; i++)
  //   for (size_t j = 0; j < queries_ins[i].size(); j++)
  //     faq << Ans_ins[i][j];
  // for (size_t i = 0; i < num_batches; i++)
  //   for (size_t j = 0; j < queries_del[i].size(); j++)
  //     faq << Ans_del[i][j];
  // faq.close();
  // return 0;
}