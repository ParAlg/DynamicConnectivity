#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include <cmath>
#include <cstddef>
#include <dycon/helpers/graph_utils.hpp>
#include <dycon/helpers/parse_command_line.hpp>
#include <dycon/localTree/CWN.hpp>
#include <dycon/localTree/SCCWN.hpp>
#include <fstream>
#include <parlay/internal/get_time.h>

#include <dynamic_graph/dynamic_connectivity.hpp>
using vertex = size_t;
using utils = graph_utils<vertex>;
using edge = utils::edge;
using edges = utils::edges;

using query = utils::query;
using queries = utils::queries;
using Ans = parlay::sequence<parlay::sequence<bool>>;
void bench_uncompressed_CWN_root(size_t num_batches, size_t n,
                                 std::string ansfile,
                                 parlay::sequence<edges> &batches_ins,
                                 parlay::sequence<queries> &queries_ins,
                                 parlay::sequence<edges> &batches_del,
                                 parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking uncomressed CWN\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });
  parlay::internal::timer t;
  std::ofstream fins, fdel;
  CWN F(n);
  F.lmax = std::ceil(std::log2(n));
  t.next("start benchmarking uncompressed CWN with edges inserted to root");
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToRoot(u, v);
    }
    t.next("Insert batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_ins[i].size(); j++) {
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    t.next("Delete batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
  faq.close();
}
void bench_uncompressed_CWN_blocked(size_t num_batches, size_t n,
                                    std::string ansfile,
                                    parlay::sequence<edges> &batches_ins,
                                    parlay::sequence<queries> &queries_ins,
                                    parlay::sequence<edges> &batches_del,
                                    parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking uncomressed CWN\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });
  parlay::internal::timer t;
  std::ofstream fins, fdel;
  CWN F(n);
  F.lmax = std::ceil(std::log2(n));
  t.next("start benchmarking uncompressed CWN with edges inserted to block");
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToBlock(u, v);
    }
    t.next("Insert batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_ins[i].size(); j++) {
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    t.next("Delete batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
  faq.close();
}
void bench_compress_CWN_root(size_t num_batches, size_t n, std::string ansfile,
                             parlay::sequence<edges> &batches_ins,
                             parlay::sequence<queries> &queries_ins,
                             parlay::sequence<edges> &batches_del,
                             parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking comressed CWN with edges inserted to root\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });
  parlay::internal::timer t;
  std::ofstream fins, fdel;
  SCCWN F(n);
  F.lmax = std::ceil(std::log2(n));
  t.next("start benchmarking comressed CWN with edges inserted to root");
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToRoot(u, v);
    }
    t.next("Insert batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_ins[i].size(); j++) {
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    t.next("Delete batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
  faq.close();
}
void bench_compress_CWN_lca(size_t num_batches, size_t n, std::string ansfile,
                            parlay::sequence<edges> &batches_ins,
                            parlay::sequence<queries> &queries_ins,
                            parlay::sequence<edges> &batches_del,
                            parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking comressed CWN with edges inserted to lca\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });
  parlay::internal::timer t;
  std::ofstream fins, fdel;
  SCCWN F(n);
  F.lmax = std::ceil(std::log2(n));
  t.next("start benchmarking comressed CWN with edges inserted to lca");
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToLCA(u, v);
    }
    t.next("Insert batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_ins[i].size(); j++) {
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    t.next("Delete batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
  faq.close();
}
void bench_compress_CWN_blocked(size_t num_batches, size_t n,
                                std::string ansfile,
                                parlay::sequence<edges> &batches_ins,
                                parlay::sequence<queries> &queries_ins,
                                parlay::sequence<edges> &batches_del,
                                parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking comressed CWN with edges inserted to blocked\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });
  parlay::internal::timer t;
  std::ofstream fins, fdel;
  SCCWN F(n);
  F.lmax = std::ceil(std::log2(n));
  t.next("start benchmarking comressed CWN with edges inserted to blocked");
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToBlock(u, v);
    }
    t.next("Insert batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_ins[i].size(); j++) {
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    t.next("Delete batch #" + std::to_string(i));
    for (size_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    t.next("Answer queries #" + std::to_string(i));
  }
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
  faq.close();
}
void bench_seq_hdt(size_t num_batches, size_t n, std::string ansfile,
                   parlay::sequence<edges> &batches_ins,
                   parlay::sequence<queries> &queries_ins,
                   parlay::sequence<edges> &batches_del,
                   parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking comressed CWN with edges inserted to root\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](size_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });
  parlay::internal::timer t("");
  std::ofstream fins, fdel;
  DynamicConnectivity graph(n);
  t.next("start benchmarking sequential hdt");
  std::cout << std::endl;
  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      graph.AddEdge(UndirectedEdge(u, v));
    }
    // // t.next("Insert batch #" + std::to_string(i));
    // t.next("");
    // for (size_t j = 0; j < queries_ins[i].size(); j++)
    //   Ans_ins[i][j] =
    //       graph.IsConnected(queries_ins[i][j].first, queries_ins[i][j].second);
    // // t.next("Answer queries #" + std::to_string(i));
    // t.next("");
    std::cout << graph.space() << " ";
  }

  for (size_t i = 0; i < num_batches; i++) {
    for (size_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      graph.DeleteEdge(UndirectedEdge(u, v));
    }
    // // t.next("Delete batch #" + std::to_string(i));
    // t.next("");
    // for (size_t j = 0; j < queries_del[i].size(); j++)
    //   Ans_del[i][j] =
    //       graph.IsConnected(queries_del[i][j].first, queries_del[i][j].second);
    // // t.next("Answer queries #" + std::to_string(i));
    // t.next("");
    std::cout << graph.space() << " ";
  }
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (size_t i = 0; i < num_batches; i++)
    for (size_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
  faq.close();
}

int main(int argc, char **argv) {

  std::string usage = "Usage: Connectivity [-a #algorithms] [-b #batches] [-q "
                      "#queries/batch] <input_file> <output_file>";
  commandLine P(argc, argv, usage);
  auto IOF = P.IOFileNames();
  std::string In = IOF.first;
  std::string Out = IOF.second;
  size_t algorithms = P.getOptionIntValue("-a", 0);
  size_t num_batches = P.getOptionIntValue("-b", 10);
  size_t num_queries = P.getOptionIntValue("-q", 1000);

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

  parlay::execute_with_scheduler(1, [&]() {
    switch (algorithms) {
    case 0:
      bench_uncompressed_CWN_root(
          num_batches, n, Out + "_uncompressed_CWN_root.ans", batches_ins,
          queries_ins, batches_del, queries_del);
      bench_uncompressed_CWN_blocked(
          num_batches, n, Out + "_uncompressed_CWN_blocked.ans", batches_ins,
          queries_ins, batches_del, queries_del);
      bench_compress_CWN_root(num_batches, n, Out + "_compressed_CWN_root.ans",
                              batches_ins, queries_ins, batches_del,
                              queries_del);
      bench_compress_CWN_lca(num_batches, n, Out + "_compressed_CWN_lca.ans",
                             batches_ins, queries_ins, batches_del,
                             queries_del);
      bench_compress_CWN_blocked(
          num_batches, n, Out + "_compressed_CWN_blocked.ans", batches_ins,
          queries_ins, batches_del, queries_del);
      bench_seq_hdt(num_batches, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      break;
    case 1:
      bench_uncompressed_CWN_root(
          num_batches, n, Out + "_uncompressed_CWN_root.ans", batches_ins,
          queries_ins, batches_del, queries_del);
      break;
    case 2:
      bench_uncompressed_CWN_blocked(
          num_batches, n, Out + "_uncompressed_CWN_blocked.ans", batches_ins,
          queries_ins, batches_del, queries_del);
      break;
    case 3:
      bench_compress_CWN_root(num_batches, n, Out + "_compressed_CWN_root.ans",
                              batches_ins, queries_ins, batches_del,
                              queries_del);
      break;
    case 4:
      bench_compress_CWN_lca(num_batches, n, Out + "_compressed_CWN_lca.ans",
                             batches_ins, queries_ins, batches_del,
                             queries_del);
      break;
    case 5:
      bench_compress_CWN_blocked(
          num_batches, n, Out + "_compressed_CWN_blocked.ans", batches_ins,
          queries_ins, batches_del, queries_del);
      break;
    case 6:
      bench_seq_hdt(num_batches, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      break;
    }
  });
  return 0;
}