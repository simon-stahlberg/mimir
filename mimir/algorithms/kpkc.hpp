#if !defined(ALGORITHMS_KPKC_HPP_)
#define ALGORITHMS_KPKC_HPP_

#include <boost/dynamic_bitset.hpp>
#include <chrono>
#include <iostream>
#include <limits>
#include <vector>

namespace algorithms
{
    // Find all cliques of size k in a k-partite graph
    bool find_all_k_cliques_in_k_partite_graph(const std::chrono::high_resolution_clock::time_point end_time,
                                               const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                               const std::vector<std::vector<size_t>>& partitions,
                                               std::vector<std::vector<std::size_t>>& out_cliques);
}  // namespace algorithms

#endif  // ALGORITHMS_KPKC_HPP_
