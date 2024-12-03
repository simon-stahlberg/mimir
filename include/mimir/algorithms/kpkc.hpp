/*
 * Copyright (C) 2023 Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_ALGORITHMS_KPKC_HPP_
#define MIMIR_ALGORITHMS_KPKC_HPP_

#include <boost/dynamic_bitset.hpp>
#include <vector>

namespace mimir
{

// When a clique is found, this callback function is called. The return value indicates whether to continue.
using OnCliqueFoundCallback = std::function<bool(const std::vector<std::size_t>&)>;

// Find all cliques of size k in a k-partite graph
void find_all_k_cliques_in_k_partite_graph(const OnCliqueFoundCallback& on_clique,
                                           const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                           const std::vector<std::vector<size_t>>& partitions);

class KCliqueInKPartiteGraphConstIterator
{
private:
    const std::vector<boost::dynamic_bitset<>>& m_adjacency_matrix;
    const std::vector<std::vector<size_t>>& m_partitions;

public:
    explicit KCliqueInKPartiteGraphConstIterator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix,
                                                 const std::vector<std::vector<size_t>>& partitions) :
        m_adjacency_matrix(adjacency_matrix),
        m_partitions(partitions)
    {
    }

    /**
     * Iterators
     */
    class const_iterator
    {
    private:
        const std::vector<boost::dynamic_bitset<>>* m_adjacency_matrix;
        const std::vector<std::vector<size_t>>* m_partitions;

        std::vector<size_t> m_clique;
        size_t m_pos;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = std::vector<size_t>;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        const_iterator() : m_adjacency_matrix(nullptr), m_partitions(nullptr), m_clique(), m_pos(-1) {}
        const_iterator(const std::vector<boost::dynamic_bitset<>>& adjacency_matrix, const std::vector<std::vector<size_t>>& partitions, bool begin) :
            m_adjacency_matrix(&adjacency_matrix),
            m_partitions(&partitions)
        {
        }

        reference operator*() const { return m_clique; }

        const_iterator& operator++() {}

        const_iterator operator++(int)
        {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const const_iterator& other) const { return m_pos == other.m_pos; }

        bool operator!=(const const_iterator& other) const { return !(*this == other); }
    };

    const_iterator begin() const { return const_iterator(m_adjacency_matrix, m_partitions, true); }

    const_iterator end() const { return const_iterator(m_adjacency_matrix, m_partitions, false); }
};

}

#endif  // MIMIR_ALGORITHMS_KPKC_HPP_
