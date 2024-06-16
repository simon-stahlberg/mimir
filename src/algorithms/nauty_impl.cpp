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

#include <sstream>
#include <stdexcept>

namespace nauty_wrapper
{
GraphImpl::GraphImpl(int num_vertices) : n(num_vertices), m(SETWORDSNEEDED(n)), graph_(new graph[m * n]) { EMPTYGRAPH0(graph_, m, n); }

GraphImpl::~GraphImpl() { delete[] graph_; }

void GraphImpl::add_edge(int src, int dst)
{
    if (src >= n || dst >= n || src < 0 || dst < 0)
    {
        throw std::out_of_range("Vertex index out of range");
    }
    ADDONEEDGE0(graph_, src, dst, m);
}

std::string GraphImpl::compute_certificate() const
{
    graph canon_graph[n * m];
    int lab[n], ptn[n], orbits[n];
    static DEFAULTOPTIONS_GRAPH(options);
    options.getcanon = TRUE;
    statsblk stats;
    setword workspace[2 * m * n];

    nauty(graph_, lab, ptn, NULL, orbits, &options, &stats, workspace, 2 * m * n, m, n, canon_graph);

    std::ostringstream oss;
    for (int i = 0; i < n * m; ++i)
    {
        oss << canon_graph[i] << " ";
    }
    return oss.str();
}

}