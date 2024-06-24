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

#ifndef SRC_ALGORITHMS_NAUTY_IMPL_HPP_
#define SRC_ALGORITHMS_NAUTY_IMPL_HPP_

// Only include nauty_impl.hpp in a source file to avoid transitive includes of nauty.h.
#include <nauty.h>
#include <string>
#include <vector>

namespace nauty_wrapper
{
class GraphImpl
{
private:
    // num_vertices
    int n;
    // blocks_per_vertex
    int m;

    graph* graph_;

    void allocate_graph();
    void deallocate_graph();

public:
    explicit GraphImpl(int num_vertices);
    GraphImpl(const GraphImpl& other);
    GraphImpl& operator=(const GraphImpl& other);
    GraphImpl(GraphImpl&& other) noexcept;
    GraphImpl& operator=(GraphImpl&& other) noexcept;
    ~GraphImpl();

    void add_edge(int src, int dst);

    std::string compute_certificate(const std::vector<int>& lab, const std::vector<int>& ptn) const;

    void reset(int num_vertices);
};

}

#endif
