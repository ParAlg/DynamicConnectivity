#pragma once
#include "graph_utils.hpp"
#include <fstream>
#include <vector>

using namespace std;
using vertex = size_t;
using utils = graph_utils<vertex>;
using edge = utils::edge;
using edges = utils::edges;
using query = utils::query;
using queries = utils::queries;
using Pair = std::pair<size_t, size_t>;
template <typename vertex> struct union_find {
  parlay::sequence<vertex> parents;
  union_find(vertex n)
      : parents(parlay::tabulate<vertex>(n, [&](vertex i) { return i; })) {}
  vertex find(vertex x) {
    return parents[x] == x ? x : (parents[x] = find(parents[x]));
  }
  void link(vertex u, vertex v) {
    vertex x = find(u);
    vertex y = find(v);
    if (x == y)
      return;
    if (x < y)
      parents[x] = y;
    else
      parents[y] = x;
  }
};
inline void static_connectivity(parlay::sequence<edge> &E,
                                parlay::sequence<vertex> &V, size_t &n,
                                size_t &m, parlay::sequence<query> &Q,
                                parlay::sequence<bool> &Ans) {

  size_t maxID = utils::num_vertices(E);
  union_find<vertex> UF(maxID);
  for (size_t i = 0; i < m; i++)
    UF.link(E[i].first, E[i].second);
  if (!m) {
    parlay::parallel_for(0, Q.size(), [&](size_t i) { Ans[i] = 0; });
    return;
  }
  for (size_t i = 0; i < Q.size(); i++)
    Ans[i] = (UF.find(Q[i].first) == UF.find(Q[i].second)) ? true : false;
}
