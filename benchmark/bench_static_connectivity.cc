#include <dycon/helpers/graph_utils.hpp>
#include <dycon/helpers/parse_command_line.hpp>
#include <dycon/helpers/union_find.hpp>
#include <parlay/internal/get_time.h>
#include <string>

using namespace std;
void test(parlay::sequence<edges> &instances, parlay::sequence<queries> &q,
          uint32_t num_batches, ofstream &faq) {
  for (uint32_t i = 0; i < num_batches; i++) {

    auto E = instances[i];
    auto Q = q[i];
    uint32_t m = E.size();
    auto V = utils::avail_vertices(E, utils::num_vertices(E));
    // cout << utils::num_vertices(E) << endl;
    uint32_t n = V.size();
    parlay::sequence<bool> Ans(Q.size());
    parlay::internal::timer t;
    t.start();
    static_connectivity(E, V, n, m, Q, Ans);
    t.next("union find ");
    for (uint32_t j = 0; j < Q.size(); j++)
      faq << Ans[j];
  }
}
int main(int argc, char **argv) {
  string usage = "Usage: Connectivity [-b #batches] [-q #queries/batch] "
                 "<input_file> <output_file>";
  commandLine P(argc, argv, usage);

  auto IOF = P.IOFileNames();
  string In = IOF.first;
  string Out = IOF.second;
  uint32_t num_batches = P.getOptionIntValue("-b", 10);
  uint32_t num_queries = P.getOptionIntValue("-q", 1000);
  // cout << In << endl << Out << endl;
  auto G = utils::break_sym_graph_from_bin(In);
  vertex n = G.size();
  auto E =
      parlay::random_shuffle(parlay::remove_duplicates(utils::to_edges(G)));
  vertex m = E.size();
  // cout << n << " " << m << " -b " << num_batches << " -q " << num_queries <<
  // endl;

  auto batch_size = parlay::tabulate(
      num_batches + 1, [&](uint32_t i) { return m / num_batches * i; });
  batch_size[num_batches] = m;

  // Insertion only graph instances and queries
  auto instances_ins = parlay::tabulate(num_batches, [&](uint32_t i) {
    return parlay::to_sequence(E.cut(batch_size[0], batch_size[i + 1]));
  });
  auto batches_ins = parlay::tabulate(num_batches, [&](uint32_t i) {
    return parlay::to_sequence(E.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_ins =
      utils::generate_CC_queries(num_batches, batches_ins, n, num_queries);

  // Deletion only graph instances and queries.
  auto E_ = parlay::random_shuffle(E);
  auto instances_del = parlay::tabulate(num_batches, [&](uint32_t i) {
    return parlay::to_sequence(
        E_.cut(batch_size[i + 1], batch_size[num_batches]));
  });
  auto batches_del = parlay::tabulate(num_batches, [&](uint32_t i) {
    return parlay::to_sequence(E_.cut(batch_size[i], batch_size[i + 1]));
  });
  auto queries_del =
      utils::generate_CC_queries(num_batches, batches_del, n, num_queries);

  // std::ofstream fout;
  // fout.open("../../../utils/static_connectivity.txt");
  // auto sorted_edges = parlay::sort(instances_del[0]);
  // utils::write_edges_to_file(sorted_edges, fout);
  // fout.close();

  ofstream faq;
  faq.open(Out);
  if (!faq.is_open()) {
    cout << "cannot open output file\n";
    std::abort();
  }
  test(instances_ins, queries_ins, num_batches, faq);
  test(instances_del, queries_del, num_batches, faq);
  faq.close();

  return 0;
}