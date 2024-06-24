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

#include "mimir/algorithms/nauty.hpp"

#include "nauty_impl.hpp"

namespace nauty_wrapper
{

Graph::Graph() : m_impl(std::make_unique<GraphImpl>(0)) {}

Graph::Graph(int num_vertices) : m_impl(std::make_unique<GraphImpl>(num_vertices)) {}

Graph::Graph(const Graph& other) : m_impl(std::make_unique<GraphImpl>(*other.m_impl)) {}

Graph& Graph::operator=(const Graph& other)
{
    if (this != &other)
    {
        m_impl = std::make_unique<GraphImpl>(*other.m_impl);
    }
    return *this;
}

Graph::Graph(Graph&& other) noexcept : m_impl(std::move(other.m_impl)) {}

Graph& Graph::operator=(Graph&& other) noexcept
{
    if (this != &other)
    {
        std::swap(m_impl, other.m_impl);
    }
    return *this;
}

Graph::~Graph() = default;

void Graph::add_edge(int src, int dst) { m_impl->add_edge(src, dst); }

std::string Graph::compute_certificate(const std::vector<int>& lab, const std::vector<int>& ptn) const { return m_impl->compute_certificate(lab, ptn); }

void Graph::reset(int num_vertices) { m_impl->reset(num_vertices); }

}