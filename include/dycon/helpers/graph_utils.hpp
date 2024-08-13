// Copied from parlaylib with some modification
#pragma once
#include <iostream>
#include <parlay/io.h>
#include <parlay/primitives.h>
#include <parlay/sequence.h>
#include <string>

template <typename vertex> struct graph_utils {
  using edge = std::pair<vertex, vertex>;
  using edges = parlay::sequence<edge>;
  using vertices = parlay::sequence<vertex>;
  using graph = parlay::sequence<vertices>;
  using query = std::pair<vertex, vertex>;
  using queries = parlay::sequence<query>;

  template <typename wtype>
  using weighted_vertices = parlay::sequence<std::pair<vertex, wtype>>;
  template <typename wtype>
  using weighted_graph = parlay::sequence<weighted_vertices<wtype>>;

  template <typename wtype>
  using weighted_edge = std::tuple<vertex, vertex, wtype>;
  template <typename wtype>
  using weighted_edges = parlay::sequence<weighted_edge<wtype>>;

  using element = std::pair<int, float>;
  using row = parlay::sequence<element>;
  using sparse_matrix = parlay::sequence<row>;

  // transpose a directed graph
  // i.e. generate the backward edges for every forward edges
  static graph transpose(const graph &G) {
    auto pairs = flatten(parlay::delayed::tabulate(G.size(), [&](vertex i) {
      return map(G[i], [=](auto ngh) { return std::pair(ngh, i); }, 1000);
    }));
    return group_by_index(pairs, G.size());
  }

  // symmetrize and remove self edges from a graph
  static graph symmetrize(const graph &G) {
    auto GT = transpose(G);
    return parlay::tabulate(G.size(), [&](long i) {
      return parlay::filter(
          parlay::remove_duplicates(parlay::append(GT[i], G[i])),
          [=](auto j) { return i != j; });
    });
  }

  // symmetrize and remove self edges from an edge sequence
  static graph symmetrize(const edges &Ein, long n) {
    auto E = filter(Ein, [](auto e) { return e.first != e.second; });
    auto ET = map(E, [](auto e) { return std::pair(e.second, e.first); });
    auto G = group_by_index(E, n);
    auto GT = group_by_index(ET, n);
    return parlay::tabulate(n, [&](long i) {
      return parlay::remove_duplicates(append(G[i], GT[i]));
    });
  }

  static edges to_edges(const graph &G) {
    return flatten(parlay::tabulate(G.size(), [&](vertex u) {
      return map(G[u], [=](vertex v) { return std::pair(u, v); });
    }));
  }

  // adds random weights so (u,v) and (v,u) have same weight
  template <typename wtype>
  static weighted_edges<wtype> add_weights(const edges &E) {
    parlay::random_generator gen;
    std::uniform_real_distribution<wtype> dis(0.0, 1.0);
    return parlay::tabulate(E.size(), [&](long i) {
      auto [u, v] = E[i];
      auto r = gen[std::min(u, v)][std::max(u, v)];
      return weighted_edge<wtype>(u, v, dis(r));
    });
  }

  template <typename wtype>
  static auto get_distribution(wtype minw, wtype maxw) {
    if constexpr (std::is_integral_v<wtype>)
      return std::uniform_int_distribution<wtype>(minw, maxw);
    else
      return std::uniform_real_distribution<wtype>(minw, maxw);
  }

  // adds random weights so (u,v) and (v,u) have same weight
  template <typename wtype>
  static weighted_graph<wtype> add_weights(const graph &G, wtype minw,
                                           wtype maxw) {
    parlay::random_generator gen;
    auto dis = get_distribution(minw, maxw);
    return parlay::tabulate(G.size(), [&](vertex u) {
      return parlay::map(G[u], [&](vertex v) {
        auto r = gen[std::min(u, v)][std::max(u, v)];
        return std::pair(v, dis(r));
      });
    });
  }

  static long num_vertices(const edges &E) {
    // std::cout << E.size() << std::endl;
    return reduce(map(E, [](auto e) { return std::max(e.first, e.second); }),
                  parlay::maximum<vertex>()) +
           1;
  }

  static sparse_matrix to_normalized_matrix(const graph &G) {
    auto column_counts = histogram_by_index(flatten(G), G.size());
    return parlay::map(G, [&](auto &nghs) {
      return parlay::map(
          nghs, [&](vertex c) { return element(c, 1.0 / column_counts[c]); },
          100);
    });
  }

  template <typename gen>
  static edge rmat_edge(int logn, double a, double b, double c, double r,
                        gen g) {
    if (logn == 0)
      return edge(0, 0);
    else {
      long no2 = 1 << (logn - 1);
      r = g();
      if (r < a) {
        auto [u, v] = rmat_edge(logn - 1, a, b, c, r / a, g);
        return edge(u, v);
      } else if (r < a + b) {
        auto [u, v] = rmat_edge(logn - 1, a, b, c, (r - a) / b, g);
        return edge(u, v + no2);
      } else if (r < a + b + c) {
        auto [u, v] = rmat_edge(logn - 1, a, b, c, (r - a - b) / c, g);
        return edge(u + no2, v);
      } else {
        auto [u, v] =
            rmat_edge(logn - 1, a, b, c, (r - a - b - c) / (1 - a + b + c), g);
        return edge(u + no2, v + no2);
      }
    }
  }

  static edges rmat_edges_(int logn, long m, double a = .5, double b = .15,
                           double c = .15) {
    parlay::random_generator gen;
    std::uniform_real_distribution<double> dis(0.0, 1.0);
    return remove_duplicates(parlay::tabulate(m, [&](long i) {
      auto r = gen[i];
      return rmat_edge(logn, a, b, c, 0.0, [&]() { return dis(r); });
    }));
  }

  static edges rmat_edges(int n, long m, double a = .5, double b = .15,
                          double c = .15) {
    return rmat_edges_((int)round(log2(n)), m, a, b, c);
  }

  static graph rmat_graph(long n, long m, double a = .5, double b = .15,
                          double c = .15) {
    int logn = round(log2(n));
    auto edges = rmat_edges_(logn, m, a, b, c);
    return parlay::group_by_index(edges, 1 << logn);
  }

  static graph rmat_symmetric_graph(long n, long m, double a = .5,
                                    double b = .15, double c = .15) {
    int logn = round(log2(n));
    auto edges = rmat_edges_(logn, m / 2, a, b, c);
    return symmetrize(edges, 1 << logn);
  }

  static graph grid_graph(long n) {
    vertex sqn = sqrt(n);
    parlay::sequence<vertex> offsets(
        {-1 - sqn, -sqn, 1 - sqn, -1, 1, -1 + sqn, sqn, 1 + sqn});
    return parlay::tabulate(sqn * sqn, [&](vertex u) {
      auto nghs = map(offsets, [&](vertex o) { return u + o; });
      return filter(nghs, [=](vertex v) {
        return (v >= 0 && v < sqn * sqn && abs(u % sqn - v % sqn) < 2);
      });
    });
  }

  static void print_graph_stats(const graph &G) {
    long num_edges = reduce(map(G, parlay::size_of()));
    long max_degree =
        reduce(map(G, parlay::size_of()), parlay::maximum<size_t>());
    std::cout << "num vertices = " << G.size() << std::endl;
    std::cout << "num edges    = " << num_edges << std::endl;
    std::cout << "max degree   = " << max_degree << std::endl;
  }

  static void print_graph_stats(const edges &E, long n) {
    auto ET = parlay::delayed::map(
        E, [](auto e) { return std::pair(e.second, e.first); });
    auto Ef = remove_duplicates(append(E, ET));
    auto e1 = parlay::delayed::map(Ef, [](auto e) { return e.first; });
    long max_degree = parlay::reduce(parlay::histogram_by_index<long>(e1, n),
                                     parlay::maximum<long>());
    std::cout << "num vertices = " << n << std::endl;
    std::cout << "num edges    = " << E.size() << std::endl;
    std::cout << "max degree   = " << max_degree << std::endl;
  }

  static graph read_graph_from_file_pbbs(const std::string &filename) {
    auto str = parlay::file_map(filename);
    auto tokens = parlay::tokens(str, [](char c) { return c == '\n'; });
    long n = parlay::chars_to_long(tokens[1]);
    long m = parlay::chars_to_long(tokens[2]);
    if (tokens.size() != n + m + 3) {
      std::cout << "bad file format" << std::endl;
      return graph();
    }
    auto offsets = parlay::tabulate(
        n, [&](long i) { return parlay::chars_to_double(tokens[i + 3]); });
    auto edges = parlay::tabulate(m, [&](long i) {
      return (vertex)parlay::chars_to_double(tokens[i + n + 3]);
    });
    return parlay::tabulate(n, [&](long i) {
      long end = (i == n - 1) ? m : offsets[i + 1];
      return to_sequence(edges.cut(offsets[i], end));
    });
  }

  static graph read_graph_from_file(const std::string &filename) {
    auto str = parlay::file_map(filename);
    auto tokens =
        parlay::tokens(str, [](char c) { return c == '\n' || c == ' '; });
    long n = parlay::chars_to_long(tokens[0]);
    long m = parlay::chars_to_long(tokens[1]);
    if (tokens.size() != n + m + 2) {
      std::cout << "Bad file format, read_graph_from_file expects:\n"
                << "<n> <m> <degree 0> <degree 1> ... <degree n-1> <edge 0> "
                   "... <edge m-1>\n"
                << "Edges are sorted and each difference encoded with respect "
                   "to the previous one."
                << "First per vertex is encoded directly." << std::endl;
      return graph();
    }
    auto lengths = parlay::delayed::tabulate(
        n, [&](long i) { return parlay::chars_to_double(tokens[i + 2]); });
    auto edges = parlay::tabulate(m, [&](long i) {
      return (vertex)parlay::chars_to_double(tokens[i + n + 2]);
    });
    auto [offsets, total] = scan(lengths);
    return parlay::tabulate(n, [&, o = offsets.begin()](vertex i) {
      return scan_inclusive(edges.cut(o[i], o[i] + lengths[i]));
    });
  }

  // assumes each edge is kept in just one direction, so copied into other
  static graph read_symmetric_graph_from_file(const std::string &filename) {
    auto G = read_graph_from_file(filename);
    auto GT = transpose(G);
    return parlay::tabulate(G.size(), [&](long i) {
      return parlay::append(parlay::sort(GT[i]), G[i]);
    });
  }

  static void write_graph_to_file(const graph &G, const std::string &filename) {
    using lseq = parlay::sequence<long>;
    auto lengths = parlay::map(G, [](auto &nghs) { return (long)nghs.size(); });
    auto edges = parlay::flatten(parlay::tabulate(G.size(), [&](long i) {
      auto nghs = sort(G[i]);
      return parlay::tabulate(nghs.size(), [&](long j) -> long {
        return j == 0 ? nghs[0] : nghs[j] - nghs[j - 1];
      });
    }));
    auto all = flatten(
        parlay::sequence<lseq>({lseq(1, (long)G.size()),
                                lseq(1, (long)edges.size()), lengths, edges}));
    auto newline = parlay::chars(1, '\n');
    parlay::chars_to_file(
        parlay::flatten(map(
            all, [&](long v) { return append(parlay::to_chars(v), newline); })),
        filename);
  }

  static void write_symmetric_graph_to_file(const graph &G,
                                            const std::string &filename) {
    auto GR = parlay::tabulate(G.size(), [&](vertex u) {
      return parlay::filter(G[u], [&](vertex v) { return v > u; });
    });
    write_graph_to_file(GR, filename);
  }

  static graph break_sym_graph_from_bin(const std::string &filename) {
    std::ifstream ifs(filename);
    if (!ifs.is_open()) {
      std::cerr << "Error: Cannot open file " << filename << '\n';
      abort();
    }
    size_t num_vertices, num_edges, sizes;
    ifs.read(reinterpret_cast<char *>(&num_vertices), sizeof(size_t));
    ifs.read(reinterpret_cast<char *>(&num_edges), sizeof(size_t));
    ifs.read(reinterpret_cast<char *>(&sizes), sizeof(size_t));
    assert(sizes == (num_vertices + 1) * 8 + num_edges * 4 + 3 * 8);
    parlay::sequence<uint64_t> offset(num_vertices + 1);
    parlay::sequence<uint32_t> edge(num_edges);
    ifs.read(reinterpret_cast<char *>(offset.begin()), (num_vertices + 1) * 8);
    ifs.read(reinterpret_cast<char *>(edge.begin()), num_edges * 4);
    if (ifs.peek() != EOF) {
      std::cerr << "Error: Bad data\n";
      abort();
    }
    ifs.close();
    // std::cout << num_vertices << std::endl
    //           << num_edges << std::endl
    //           << sizes << std::endl;
    auto edges =
        parlay::tabulate(num_edges, [&](vertex i) { return (vertex)edge[i]; });
    return parlay::tabulate(num_vertices, [&](vertex i) {
      return parlay::filter(
          parlay::to_sequence(edges.cut(offset[i], offset[i + 1])),
          [=](auto v) { return i < v; });
    });
  }
  static void write_edges_to_filename(const edges &E,
                                      const std::string &filename, vertex &n,
                                      vertex &m) {
    std::ofstream fout;
    fout.open(filename);
    fout << n << " " << m << std::endl;
    for (int i = 0; i < E.size(); i++)
      fout << E[i].first << " " << E[i].second << std::endl;
    fout.close();
  }
  static void write_edges_to_file(const edges &E, std::ofstream &fout) {
    fout << num_vertices(E) << " " << E.size() << std::endl;
    for (int i = 0; i < E.size(); i++)
      fout << E[i].first << " " << E[i].second << std::endl;
  }
  static void decode_edges(const edges &E, const std::string &filename) {
    auto E_ = parlay::map(E, [](auto &e) {
      return parlay::to_chars(std::to_string(e.first) + " " +
                              std::to_string(e.second) + "\n");
    });
    parlay::chars_to_file(parlay::flatten(E_), filename);
  }
  static parlay::sequence<vertex> avail_vertices(const edges &E, vertex n) {
    parlay::sequence<bool> flags(n, false);
    // std::cout << "avail_vertices: " << n << std::endl;
    parlay::parallel_for(0, E.size(), [&](auto i) {
      flags[E[i].first] = true;
      flags[E[i].second] = true;
    });
    return parlay::filter(parlay::iota<vertex>(n),
                          [&](auto i) { return flags[i]; });
  }
  static parlay::sequence<queries>
  generate_CC_queries(size_t &num_batches, parlay::sequence<edges> &E,
                      size_t &n, size_t &num_queries) {
    return parlay::tabulate(num_batches, [&](size_t i) {
      auto E_ = E[i];
      auto candidate_vertices = avail_vertices(E_, n);
      // std::cout << candidate_vertices.size() << std::endl;
      queries Q(num_queries);
      parlay::parallel_for(0, Q.size(), [&](size_t j) {
        Q[j].first = // 50 percent non-connect pair
            j % 2 ? E_[parlay::hash64(j) % E_.size()].first
                  : candidate_vertices[parlay::hash64(
                                           j + candidate_vertices.size()) %
                                       candidate_vertices.size()];
        Q[j].second = // 50 percent non-connect pair
            j % 2 ? E_[parlay::hash64(j) % E_.size()].second
                  : candidate_vertices[parlay::hash64(j) %
                                       candidate_vertices.size()];
      });
      return Q;
    });
  }
};
