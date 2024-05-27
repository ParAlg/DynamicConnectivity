#include "SCCWN.hpp"
#include "CWN.hpp"
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
  bool use_compression = P.getOptionIntValue("-c", 0);
  bool blocked_insert = P.getOptionIntValue("-i", 0);

  std::cout << "INITIALIZING INPUT GRAPH " << In << std::endl;

  auto G = utils::break_sym_graph_from_bin(In);
  vertex n = G.size();

  auto E = parlay::random_shuffle(parlay::remove_duplicates(utils::to_edges(G)));
  vertex m = E.size();

  std::cout << "n=" << n << std::endl;
  std::cout << "m=" << m << std::endl;

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

  bool do_queries = true;

  t.start();
  std::cout << "Using cluster forest: " << (use_compression ? "COMPRESSED" : "UNCOMPRESSED") << std::endl;
  std::cout << "Insertions: " << (blocked_insert ? "BLOCKED" : "ROOT") << std::endl;

  if (use_compression) {
    SCCWN F(n);
    F.lmax = std::ceil(std::log2(n));
    if (blocked_insert) F.blocked_insert = true;
    assert(F.lmax < 64);
    std::cout << std::endl << "Space: " << F.space()/1000000 << " MB" << std::endl;
    t.next("Stat collecting time");

    for (size_t i = 0; i < num_batches; i++) { // DO INSERTIONS
      std::cout << std::endl << "INSERTING BATCH " << i << std::endl;
      for (size_t j = 0; j < batches_ins[i].size(); j++) {
        long u = batches_ins[i][j].first;
        long v = batches_ins[i][j].second;
        F.insert(u, v);
      }
      t.next("Insert batch #" + std::to_string(i));

      if (do_queries) {
        std::cout << "Doing " << queries_ins[i].size() << " queries." << std::endl;
        for (size_t j = 0; j < queries_ins[i].size(); j++) {
          Ans_ins[i][j] = F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
        }
        t.next("Answer queries #" + std::to_string(i));
      }

      std::cout << std::endl; F.print_cg_sizes();
      std::cout << std::endl << "Space: " << F.space()/1000000 << " MB" << std::endl;
      // F.run_stat("./", true, false, false);
      t.next("Stat collecting time");
    }

    for (size_t i = 0; i < num_batches; i++) { // DO DELETIONS
      std::cout << std::endl << "DELETING BATCH " << i << std::endl;
      for (size_t j = 0; j < batches_del[i].size(); j++) {
        long u = batches_del[i][j].first;
        long v = batches_del[i][j].second;
        F.remove(u, v);
      }
      t.next("Delete batch #" + std::to_string(i));

      if (do_queries) {
        std::cout << "Doing " << queries_del[i].size() << " queries." << std::endl;
        for (size_t j = 0; j < queries_del[i].size(); j++) {
          Ans_del[i][j] = F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
        }
        t.next("Answer queries #" + std::to_string(i));
      }

      std::cout << std::endl; F.print_cg_sizes();
      std::cout << std::endl << "Space: " << F.space()/1000000 << " MB" << std::endl;
      // F.run_stat("./", true, false, false);
      t.next("Stat collecting time");
    }
  }
  
  else {
    CWN F(n);
    F.lmax = std::ceil(std::log2(n));
    if (blocked_insert) F.blocked_insert = true;
    assert(F.lmax < 64);
    std::cout << std::endl << "Space: " << F.space()/1000000 << " MB" << std::endl;
    t.next("Stat collecting time");

    for (size_t i = 0; i < num_batches; i++) { // DO INSERTIONS
      std::cout << std::endl << "INSERTING BATCH " << i << std::endl;
      for (size_t j = 0; j < batches_ins[i].size(); j++) {
        long u = batches_ins[i][j].first;
        long v = batches_ins[i][j].second;
        F.insert(u, v);
      }
      t.next("Insert batch #" + std::to_string(i));

      if (do_queries) {
        std::cout << "Doing " << queries_ins[i].size() << " queries." << std::endl;
        for (size_t j = 0; j < queries_ins[i].size(); j++) {
          Ans_ins[i][j] = F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
        }
        t.next("Answer queries #" + std::to_string(i));
      }

      std::cout << std::endl; F.print_cg_sizes();
      std::cout << std::endl << "Space: " << F.space()/1000000 << " MB" << std::endl;
      // F.run_stat("./", true, false, false);
      t.next("Stat collecting time");
    }

    for (size_t i = 0; i < num_batches; i++) { // DO DELETIONS
      std::cout << std::endl << "DELETING BATCH " << i << std::endl;
      for (size_t j = 0; j < batches_del[i].size(); j++) {
        long u = batches_del[i][j].first;
        long v = batches_del[i][j].second;
        F.remove(u, v);
      }
      t.next("Delete batch #" + std::to_string(i));

      if (do_queries) {
        std::cout << "Doing " << queries_del[i].size() << " queries." << std::endl;
        for (size_t j = 0; j < queries_del[i].size(); j++) {
          Ans_del[i][j] = F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
        }
        t.next("Answer queries #" + std::to_string(i));
      }

      std::cout << std::endl; F.print_cg_sizes();
      std::cout << std::endl << "Space: " << F.space()/1000000 << " MB" << std::endl;
      // F.run_stat("./", true, false, false);
      t.next("Stat collecting time");
    }
  }

  return 0;
}