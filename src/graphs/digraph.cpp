/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
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

#include "mimir/graphs/digraph.hpp"

#include <cassert>
#include <stdexcept>

namespace mimir
{
Digraph::Digraph(bool is_directed) : m_num_vertices(0), m_num_edges(0), m_is_directed(is_directed), m_forward_successors(), m_backward_successors() {}

Digraph::Digraph(int num_vertices, bool is_directed) :
    m_num_vertices(num_vertices),
    m_num_edges(0),
    m_is_directed(is_directed),
    m_forward_successors(std::vector<std::vector<int>>(m_num_vertices, std::vector<int>())),
    m_backward_successors(std::vector<std::vector<int>>(m_num_vertices, std::vector<int>()))
{
}

void Digraph::add_edge(int src, int dst)
{
    if (src >= m_num_vertices || dst >= m_num_vertices || src < 0 || dst < 0)
    {
        throw std::out_of_range("Source or destination vertex out of range");
    }
    m_forward_successors.at(src).push_back(dst);
    m_backward_successors.at(dst).push_back(src);
    if (!m_is_directed)
    {
        m_forward_successors.at(dst).push_back(src);
        m_backward_successors.at(src).push_back(dst);
    }
}

void Digraph::reset(int num_vertices, bool is_directed)
{
    for (int i = 0; i < m_num_vertices; ++i)
    {
        m_forward_successors.at(i).clear();
        m_backward_successors.at(i).clear();
    }
    // The '.resize' procedure calls the destructor on the truncated vectors,
    // which frees memory. To avoid memory allocations in the future, we keep
    // track of the intended size of these two vectors via 'm_num_vertices'.
    if (num_vertices > m_num_vertices)
    {
        m_forward_successors.resize(num_vertices, std::vector<int>());
        m_backward_successors.resize(num_vertices, std::vector<int>());
    }
    m_num_vertices = num_vertices;
    m_num_edges = 0;
    m_is_directed = is_directed;
}

void Digraph::to_nauty_graph(nauty_wrapper::Graph& out_graph) const
{
    out_graph.reset(m_num_vertices);

    for (int src = 0; src < m_num_vertices; ++src)
    {
        for (const int dst : m_forward_successors.at(src))
        {
            out_graph.add_edge(src, dst);
        }
    }
}

int Digraph::get_num_vertices() const { return m_num_vertices; }

int Digraph::get_num_edges() const { return m_num_edges; }

std::span<const std::vector<int>> Digraph::get_forward_successors() const
{
    return { m_forward_successors.begin(), m_forward_successors.begin() + get_num_vertices() };
}

std::span<const std::vector<int>> Digraph::get_backward_successors() const
{
    return { m_backward_successors.begin(), m_backward_successors.begin() + get_num_vertices() };
}

}
