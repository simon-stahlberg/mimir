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

#include "nauty_impl.hpp"

#include "mimir/common/printers.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <stdexcept>

namespace nauty_wrapper
{
using mimir::operator<<;

void GraphImpl::allocate_graph()
{
    graph_ = new graph[m * n];
    EMPTYGRAPH0(graph_, m, n);
}

void GraphImpl::deallocate_graph()
{
    delete[] graph_;
    graph_ = nullptr;
}

GraphImpl::GraphImpl(int num_vertices) : n(num_vertices), m(SETWORDSNEEDED(n)), graph_() { allocate_graph(); }

GraphImpl::GraphImpl(const GraphImpl& other) : n(other.n), m(other.m), graph_(nullptr)
{
    allocate_graph();
    std::copy(other.graph_, other.graph_ + m * n, graph_);
}

GraphImpl& GraphImpl::operator=(const GraphImpl& other)
{
    if (this != &other)
    {
        deallocate_graph();
        n = other.n;
        m = other.m;
        allocate_graph();
        std::copy(other.graph_, other.graph_ + m * n, graph_);
    }
    return *this;
}

GraphImpl::GraphImpl(GraphImpl&& other) noexcept : n(other.n), m(other.m), graph_(other.graph_) { other.graph_ = nullptr; }

GraphImpl& GraphImpl::operator=(GraphImpl&& other) noexcept
{
    if (this != &other)
    {
        deallocate_graph();
        n = other.n;
        m = other.m;
        graph_ = other.graph_;
        other.graph_ = nullptr;
    }
    return *this;
}

GraphImpl::~GraphImpl() { delete[] graph_; }

void GraphImpl::add_edge(int src, int dst)
{
    if (src >= n || dst >= n || src < 0 || dst < 0)
    {
        throw std::out_of_range("Vertex index out of range");
    }
    ADDONEARC0(graph_, src, dst, m);
}

std::string GraphImpl::compute_certificate(const std::vector<int>& lab_, const std::vector<int>& ptn_) const
{
    if (static_cast<int>(lab_.size()) != n || static_cast<int>(ptn_.size()) != n)
    {
        throw std::out_of_range("lab_ or ptn_ is incompatibl with number of vertices in the graph.");
    }

    graph canon_graph[m * n];
    EMPTYGRAPH0(canon_graph, m, n);

    int lab[n], ptn[n], orbits[n];
    std::copy(lab_.begin(), lab_.end(), lab);
    std::copy(ptn_.begin(), ptn_.end(), ptn);

    static DEFAULTOPTIONS_GRAPH(options);
    options.defaultptn = FALSE;
    options.getcanon = TRUE;
    options.digraph = FALSE;
    options.writeautoms = FALSE;
    statsblk stats;

    densenauty(graph_, lab, ptn, orbits, &options, &stats, m, n, canon_graph);

    std::ostringstream oss;
    for (int i = 0; i < n * m; ++i)
    {
        oss << canon_graph[i] << " ";
    }
    return oss.str();
}

void GraphImpl::reset(int num_vertices)
{
    const auto n_new = num_vertices;
    const auto m_new = SETWORDSNEEDED(n_new);

    if (m_new * n_new >= m * n)
    {
        delete[] graph_;
        graph_ = new graph[m_new * n_new];
    }

    m = m_new;
    n = n_new;
    EMPTYGRAPH0(graph_, m, n);
}
}
