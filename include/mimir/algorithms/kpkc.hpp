#ifndef MIMIR_ALGORITHMS_KPKC_HPP_
#define MIMIR_ALGORITHMS_KPKC_HPP_

#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace mimir
{
// Find all cliques of size k in a k-partite graph
void find_all_k_cliques_in_k_partite_graph(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                           const std::vector<std::vector<size_t>>& partitions,
                                           std::vector<std::vector<std::size_t>>& out_cliques);
}

#endif  // MIMIR_ALGORITHMS_KPKC_HPP_
