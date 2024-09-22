#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "parlay/sequence.h"
#include <cmath>
#include <cstdint>
#include <dycon/helpers/graph_utils.hpp>
#include <dycon/helpers/parse_command_line.hpp>
#include <dycon/localTree/CWN.hpp>
#include <dycon/localTree/SCCWN.hpp>
#include <fstream>
#include <ios>
#include <parlay/internal/get_time.h>

#include <dynamic_graph/dynamic_connectivity.hpp>
#include <string>
using vertex = uint32_t;
using utils = graph_utils<vertex>;
using edge = utils::edge;
using edges = utils::edges;

using query = utils::query;
using queries = utils::queries;
using Ans = parlay::sequence<parlay::sequence<bool>>;
void report(double time, std::string prefix, std::string suffix = "\n") {
#ifndef MEM_USE
  std::ios::fmtflags cout_settings = std::cout.flags();
  std::cout.precision(4);
  std::cout << std::fixed;
  if (prefix.length() > 0)
    std::cout << prefix << ": ";
  std::cout << time;
  std::cout.flags(cout_settings);
#endif
  if (suffix == "\n")
    std::cout << std::endl;
  else
    std::cout << suffix;
}
void bench_compress_CWN_root(uint32_t num_batches, uint32_t n,
                             std::string ansfile,
                             parlay::sequence<edges> &batches_ins,
                             parlay::sequence<queries> &queries_ins,
                             parlay::sequence<edges> &batches_del,
                             parlay::sequence<queries> &queries_del) {
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](uint32_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](uint32_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });

  std::ofstream fins, fdel;

  std::vector<double> ins_t;
  double ins_total = 0.0;
  std::vector<double> del_t;
  double del_total = 0.0;
  std::vector<double> query_t;
  double query_total = 0.0;

  std::cout << "start benchmarking comressed CWN with edges inserted to root"
            << std::endl;
  parlay::internal::timer t;
  SCCWN F(n);
  F.lmax = std::ceil(std::log2(n));
  ins_total = t.stop();

  for (uint32_t i = 0; i < num_batches; i++) {
    t.start();
    for (uint32_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToRoot(u, v);
    }
    ins_t.push_back(t.stop());
    std::cout << 1.0 / num_batches * (i + 1) * 100 << "% insertion performed ";
    report(ins_t.back(), "");
    t.start();
    for (uint32_t j = 0; j < queries_ins[i].size(); j++)
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    query_t.push_back(t.stop());
    std::cout << 1.0 / num_batches / 2 * (i + 1) * 100 << "% query performed ";
    report(query_t.back(), "");
#ifdef MEM_USE
    std::cout << "collecting memory usage" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << F.getMemUsage() / 1024.0 / 1024.0 / 1024.0 << " GB"
              << std::endl;
    t.next("memory usage");
#endif
  }

  for (uint32_t i = 0; i < num_batches; i++) {
    t.start();
    for (uint32_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    del_t.push_back(t.stop());
    std::cout << 1.0 / num_batches * (i + 1) * 100 << "% deletion performed ";
    report(del_t.back(), "");
    t.start();
    for (uint32_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    query_t.push_back(t.stop());
    std::cout << 1.0 / num_batches / 2 * (i + num_batches + 1) * 100
              << "% query performed ";
    report(query_t.back(), "");
#ifdef MEM_USE
    std::cout << "collecting memory usage" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << F.getMemUsage() / 1024.0 / 1024.0 / 1024.0 << " GB"
              << std::endl;
    t.next("collecting memory usage");
#endif
  }

#ifndef MEM_USE
  report(ins_total, "initialization time", "\n");
  std::cout << "insertion time by stage" << std::endl;
  for (auto i = 0; i < ins_t.size(); i++) {
    report(ins_t[i], "", " ");
    ins_total += ins_t[i];
  }
  std::cout << std::endl;
  report(ins_total, "total insertion time including initialization ", "\n");

  std::cout << "deletion time by stage " << std::endl;
  for (auto i = 0; i < del_t.size(); i++) {
    report(del_t[i], "", " ");
    del_total += del_t[i];
  }
  std::cout << std::endl;
  report(del_total, "total deletion time", "\n");

  std::cout << "query time by stage " << std::endl;
  for (auto i = 0; i < query_t.size(); i++) {
    report(query_t[i], "", " ");
    query_total += query_t[i];
  }
  std::cout << std::endl;
  report(query_total, "total query time", "\n");

  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (uint32_t i = 0; i < num_batches; i++)
    for (uint32_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (uint32_t i = 0; i < num_batches; i++) {
    for (uint32_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
    faq << std::endl << std::endl;
  }
  faq.close();
#endif
}

void bench_compress_CWN_lca(uint32_t num_batches, uint32_t n,
                            std::string ansfile,
                            parlay::sequence<edges> &batches_ins,
                            parlay::sequence<queries> &queries_ins,
                            parlay::sequence<edges> &batches_del,
                            parlay::sequence<queries> &queries_del) {
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](uint32_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](uint32_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });

  std::ofstream fins, fdel;

  std::vector<double> ins_t;
  double ins_total = 0.0;
  std::vector<double> del_t;
  double del_total = 0.0;
  std::vector<double> query_t;
  double query_total = 0.0;

  std::cout << "start benchmarking comressed CWN with edges inserted to lca"
            << std::endl;
  parlay::internal::timer t;
  SCCWN F(n);
  F.lmax = std::ceil(std::log2(n));
  ins_total = t.stop();

  for (uint32_t i = 0; i < num_batches; i++) {
    t.start();
    for (uint32_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      F.insertToLCA(u, v);
    }
    ins_t.push_back(t.stop());
    std::cout << 1.0 / num_batches * (i + 1) * 100 << "% insertion performed ";
    report(ins_t.back(), "");
    t.start();
    for (uint32_t j = 0; j < queries_ins[i].size(); j++)
      Ans_ins[i][j] =
          F.is_connected(queries_ins[i][j].first, queries_ins[i][j].second);
    query_t.push_back(t.stop());
    std::cout << 1.0 / num_batches / 2 * (i + 1) * 100 << "% query performed ";
    report(query_t.back(), "");
#ifdef MEM_USE
    std::cout << "collecting memory usage" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << F.getMemUsage() / 1024.0 / 1024.0 / 1024.0 << " GB"
              << std::endl;
    t.next("memory usage");
#endif
  }

  for (uint32_t i = 0; i < num_batches; i++) {
    t.start();
    for (uint32_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      F.remove(u, v);
    }
    del_t.push_back(t.stop());
    std::cout << 1.0 / num_batches * (i + 1) * 100 << "% deletion performed ";
    report(del_t.back(), "");
    t.start();
    for (uint32_t j = 0; j < queries_del[i].size(); j++) {
      Ans_del[i][j] =
          F.is_connected(queries_del[i][j].first, queries_del[i][j].second);
    }
    query_t.push_back(t.stop());
    std::cout << 1.0 / num_batches / 2 * (i + num_batches + 1) * 100
              << "% query performed ";
    report(query_t.back(), "");
#ifdef MEM_USE
    std::cout << "collecting memory usage" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << F.getMemUsage() / 1024.0 / 1024.0 / 1024.0 << " GB"
              << std::endl;
    t.next("collecting memory usage");
#endif
  }

#ifndef MEM_USE
  report(ins_total, "initialization time", "\n");
  std::cout << "insertion time by stage" << std::endl;
  for (auto i = 0; i < ins_t.size(); i++) {
    report(ins_t[i], "", " ");
    ins_total += ins_t[i];
  }
  std::cout << std::endl;
  report(ins_total, "total insertion time including initialization ", "\n");

  std::cout << "deletion time by stage " << std::endl;
  for (auto i = 0; i < del_t.size(); i++) {
    report(del_t[i], "", " ");
    del_total += del_t[i];
  }
  std::cout << std::endl;
  report(del_total, "total deletion time", "\n");

  std::cout << "query time by stage " << std::endl;
  for (auto i = 0; i < query_t.size(); i++) {
    report(query_t[i], "", " ");
    query_total += query_t[i];
  }
  std::cout << std::endl;
  report(query_total, "total query time", "\n");

  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (uint32_t i = 0; i < num_batches; i++)
    for (uint32_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (uint32_t i = 0; i < num_batches; i++) {
    for (uint32_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
    faq << std::endl << std::endl;
  }
  faq.close();
#endif
}

void bench_seq_hdt(uint32_t num_batches, uint32_t n, std::string ansfile,
                   parlay::sequence<edges> &batches_ins,
                   parlay::sequence<queries> &queries_ins,
                   parlay::sequence<edges> &batches_del,
                   parlay::sequence<queries> &queries_del) {
  // std::cout << "benchmarking comressed CWN with edges inserted to root\n";
  Ans Ans_ins(num_batches);
  parlay::parallel_for(0, num_batches, [&](uint32_t i) {
    Ans_ins[i].resize(queries_ins[i].size());
  });
  Ans Ans_del(num_batches);
  parlay::parallel_for(0, num_batches, [&](uint32_t i) {
    Ans_del[i].resize(queries_del[i].size());
  });

  std::cout << "start benchmarking sequential hdt" << std::endl;
  std::ofstream fins, fdel;

  std::vector<double> ins_t;
  double ins_total = 0.0;
  std::vector<double> del_t;
  double del_total = 0.0;
  std::vector<double> query_t;
  double query_total = 0.0;

  parlay::internal::timer t;
  DynamicConnectivity graph(n);
  ins_total = t.stop();

  for (uint32_t i = 0; i < num_batches; i++) {
    t.start();
    for (uint32_t j = 0; j < batches_ins[i].size(); j++) {
      long u = batches_ins[i][j].first;
      long v = batches_ins[i][j].second;
      graph.AddEdge(UndirectedEdge(u, v));
    }
    ins_t.push_back(t.stop());
    std::cout << 1.0 / num_batches * (i + 1) * 100 << "% insertion performed ";
    report(ins_t.back(), "");
    t.start();
    for (uint32_t j = 0; j < queries_ins[i].size(); j++)
      Ans_ins[i][j] =
          graph.IsConnected(queries_ins[i][j].first, queries_ins[i][j].second);
    query_t.push_back(t.stop());
    std::cout << 1.0 / num_batches / 2 * (i + 1) * 100 << "% query performed ";
    report(query_t.back(), "");
#ifdef MEM_USE
    std::cout << "collecting memory usage" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << graph.space() / 1024.0 / 1024.0 / 1024.0 << " GB" << std::endl;
    t.next("memory usage");
#endif
  }

  for (uint32_t i = 0; i < num_batches; i++) {
    t.start();
    for (uint32_t j = 0; j < batches_del[i].size(); j++) {
      long u = batches_del[i][j].first;
      long v = batches_del[i][j].second;
      graph.DeleteEdge(UndirectedEdge(u, v));
    }
    del_t.push_back(t.stop());
    std::cout << 1.0 / num_batches * (i + 1) * 100 << "% deletion performed ";
    report(del_t.back(), "");
    t.start();
    for (uint32_t j = 0; j < queries_del[i].size(); j++)
      Ans_del[i][j] =
          graph.IsConnected(queries_del[i][j].first, queries_del[i][j].second);
    query_t.push_back(t.stop());
    std::cout << 1.0 / num_batches / 2 * (i + num_batches + 1) * 100
              << "% query performed ";
    report(query_t.back(), "");
#ifdef MEM_USE
    std::cout << "collecting memory usage" << std::endl;
    std::cout << std::fixed << std::setprecision(2)
              << graph.space() / 1024.0 / 1024.0 / 1024.0 << " GB" << std::endl;
    t.next("collecting memory usage");
#endif
  }

#ifndef MEM_USE
  report(ins_total, "initialization time", "\n");
  std::cout << "insertion time by stage" << std::endl;
  for (auto i = 0; i < ins_t.size(); i++) {
    report(ins_t[i], "", " ");
    ins_total += ins_t[i];
  }
  std::cout << std::endl;
  report(ins_total, "total insertion time including initialization ", "\n");

  std::cout << "deletion time by stage " << std::endl;
  for (auto i = 0; i < del_t.size(); i++) {
    report(del_t[i], "", " ");
    del_total += del_t[i];
  }
  std::cout << std::endl;
  report(del_total, "total deletion time", "\n");

  std::cout << "query time by stage " << std::endl;
  for (auto i = 0; i < query_t.size(); i++) {
    report(query_t[i], "", " ");
    query_total += query_t[i];
  }
  std::cout << std::endl;
  report(query_total, "total query time", "\n");
  std::ofstream faq;
  faq.open(ansfile);
  if (!faq.is_open()) {
    std::cout << "cannot open output file\n";
    std::abort();
  }
  for (uint32_t i = 0; i < num_batches; i++)
    for (uint32_t j = 0; j < queries_ins[i].size(); j++)
      faq << Ans_ins[i][j];
  for (uint32_t i = 0; i < num_batches; i++) {
    for (uint32_t j = 0; j < queries_del[i].size(); j++)
      faq << Ans_del[i][j];
    faq << std::endl << std::endl;
  }
  faq.close();
#endif
}

int main(int argc, char **argv) {

  std::string usage = "Usage: Connectivity [-a #algorithms] [-b #batches] [-q "
                      "#queries/batch] <input_file> <output_file>";
  commandLine P(argc, argv, usage);
  auto IOF = P.IOFileNames();
  std::string In = IOF.first;
  std::string Out = IOF.second;
  uint32_t algorithms = P.getOptionIntValue("-a", 0);
  uint32_t num_batches = P.getOptionIntValue("-b", 10);
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
      num_batches + 1, [&](uint32_t i) { return m / num_batches * i; });
  batch_size[num_batches] = m;

  auto batches_ins = parlay::tabulate(num_batches, [&](uint32_t i) {
    return parlay::to_sequence(E.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_ins =
      utils::generate_CC_queries(num_batches, batches_ins, n, num_queries);

  auto E_ = parlay::random_shuffle(E);
  auto batches_del = parlay::tabulate(num_batches, [&](uint32_t i) {
    return parlay::to_sequence(E_.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_del =
      utils::generate_CC_queries(num_batches, batches_del, n, num_queries);
#ifndef MEM_USE
  parlay::execute_with_scheduler(1, [&]() {
#endif
    switch (algorithms) {
    case 0:
      bench_seq_hdt(num_batches, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      bench_compress_CWN_root(num_batches, n, Out + "_compressed_CWN_root.ans",
                              batches_ins, queries_ins, batches_del,
                              queries_del);
      bench_compress_CWN_lca(num_batches, n, Out + "_compressed_CWN_lca.ans",
                             batches_ins, queries_ins, batches_del,
                             queries_del);
      break;
    case 1:
      bench_seq_hdt(num_batches, n, Out + "_seqhdt.ans", batches_ins,
                    queries_ins, batches_del, queries_del);
      break;
    case 2:
      bench_compress_CWN_root(num_batches, n, Out + "_compressed_CWN_root.ans",
                              batches_ins, queries_ins, batches_del,
                              queries_del);
      break;
    case 3:
      bench_compress_CWN_lca(num_batches, n, Out + "_compressed_CWN_lca.ans",
                             batches_ins, queries_ins, batches_del,
                             queries_del);
      break;
    }
#ifndef MEM_USE
  });
#endif
  return 0;
}