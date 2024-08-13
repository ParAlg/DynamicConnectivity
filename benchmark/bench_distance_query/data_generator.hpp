#include "parlay/internal/group_by.h"
#include "parlay/io.h"
#include "parlay/primitives.h"
#include "parlay/random.h"
#include "parlay/sequence.h"
#include "parlay/utilities.h"
#include <cstddef>
#include <dycon/helpers/graph_utils.hpp>
#include <dycon/helpers/parse_command_line.hpp>
#include <string>
#include <tuple>
using vertex = size_t;
using utils = graph_utils<vertex>;
using edge = graph_utils<vertex>::edge;
using edges = graph_utils<vertex>::edges;
using graph_info = std::tuple<edges, size_t, size_t>;
inline graph_info graph_decoder(std::string filename) {
  auto G = utils::break_sym_graph_from_bin(filename);
  auto E =
      parlay::random_shuffle(parlay::remove_duplicates(utils::to_edges(G)));
  std::cout << G.size() << " " << E.size() << std::endl;
  //   utils::decode_edges(E, "output.txt");
  return std::make_tuple(parlay::random_shuffle(E), G.size(), E.size());
}
inline void generate_del_batch(const edges &E, std::string filename) {
  //   auto E_ = parlay::map(E.cut(0, E.size() / 10), [](auto &e) {
  auto E_ = parlay::map(E, [](auto &e) {
    return parlay::to_chars("ED " + std::to_string(e.first) + " " +
                            std::to_string(e.second) + "\n");
  });
  parlay::chars_to_file(parlay::flatten(E_), filename);
}
inline void generate_ins_batch(const edges &E, std::string filename) {
  auto E_ = parlay::map(E, [](auto &e) {
    return parlay::to_chars("EI " + std::to_string(e.first) + " " +
                            std::to_string(e.second) + "\n");
  });
  parlay::chars_to_file(parlay::flatten(E_), filename);
}
inline void printpair(const edges &E, std::string filename) {
  auto E_ = parlay::map(E, [](auto &e) {
    return parlay::to_chars(std::to_string(e.first) + " " +
                            std::to_string(e.second) + "\n");
  });
  parlay::chars_to_file(parlay::flatten(E_), filename);
}
inline void generate_dis_query(edges &E, std::string filename,
                               size_t num_queries) {
  auto Q = parlay::tabulate(num_queries, [&](size_t i) {
    return parlay::to_chars(
        std::to_string(E[parlay::hash32(i) % E.size()].first) + " " +
        std::to_string(E[parlay::hash64(i) % E.size()].second) + "\n");
  });
  parlay::chars_to_file(parlay::flatten(Q), filename);
}