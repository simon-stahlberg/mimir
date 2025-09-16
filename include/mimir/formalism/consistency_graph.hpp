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

#ifndef MIMIR_FORMALISM_GROUNDERS_CONSISTENCY_GRAPH_HPP_
#define MIMIR_FORMALISM_GROUNDERS_CONSISTENCY_GRAPH_HPP_

#include "mimir/common/bounds.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/common/types.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"

#include <optional>
#include <sstream>
#include <vector>

namespace mimir::formalism
{
/**
 * StaticConsistencyGraph
 */

/// @brief The StaticConsistencyGraph encodes the assignments to static conditions,
/// and hence, it is an overapproximation of the actual consistency graph.
class StaticConsistencyGraph
{
public:
    /**
     * Vertex
     */

    /// @brief A vertex [parameter_index/object_index] in the consistency graph.
    class Vertex
    {
    private:
        Index m_index;
        Index m_parameter_index;
        Index m_object_index;

    public:
        Vertex(Index index, Index parameter_index, Index object_index) : m_index(index), m_parameter_index(parameter_index), m_object_index(object_index) {}

        /// @brief Return true iff the vertex is consistent with all literals in and the assignment set.
        ///
        /// The meaning of the result being true is that the edge remains consistent.
        template<IsStaticOrFluentOrDerivedTag P>
        bool consistent_literals(const LiteralList<P>& literals, const PredicateAssignmentSets<P>& predicate_assignment_sets) const;

        bool consistent_literals(const NumericConstraintList& numeric_constraints,
                                 const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                 const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets) const;

        Index get_object_if_overlap(const Term& term) const;

        Index get_index() const { return m_index; }
        Index get_parameter_index() const { return m_parameter_index; }
        Index get_object_index() const { return m_object_index; }
    };

    /**
     * Edge
     */

    /// @brief An undirected edge {src,dst} in the consistency graph.
    class Edge
    {
    private:
        Vertex m_src;
        Vertex m_dst;

    public:
        Edge(Vertex src, Vertex dst) : m_src(std::move(src)), m_dst(std::move(dst)) {}

        /// @brief Return true iff the edge is consistent with all literals in and the assignment set.
        ///
        /// The meaning is that the assignment [x/o] is a legal assignment,
        /// and therefore the vertex must be part of the consistency graph.
        /// @param literals
        /// @param consistent_vertex
        /// @return
        template<IsStaticOrFluentOrDerivedTag P>
        bool consistent_literals(const LiteralList<P>& literals, const PredicateAssignmentSets<P>& predicate_assignment_sets) const;

        bool consistent_literals(const NumericConstraintList& numeric_constraints,
                                 const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                 const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets) const;

        Index get_object_if_overlap(const Term& term) const;

        const Vertex& get_src() const { return m_src; }
        const Vertex& get_dst() const { return m_dst; }
    };

    using Vertices = std::vector<Vertex>;

public:
    /// @brief Construct a static consistency graph
    /// @param problem The problem.
    /// @param begin_parameter_index The first parameter index for which consistent assignments are represented.
    /// @param end_parameter_index The last parameter index plus one for which consistent assignments are represented.
    /// @param static_conditions The static literals for which a bindings must be found.
    ///
    /// For universal effects, we can set first and last parameter indices respectively
    /// to find consistent assignments irrespective of the action parameter bindings.
    /// More specifically:
    ///  1. for action parameters, we set first to 0 and last to 0 + arity(action)
    ///  2. for universal effects, we set first to arity(action) and last to arity(action) + arity(effect)
    StaticConsistencyGraph(const ProblemImpl& problem, Index begin_parameter_index, Index end_parameter_index, const LiteralList<StaticTag>& static_conditions);

    /// @brief Useful to compute possible substitutions for conditional effects
    static std::tuple<Vertices, std::vector<IndexList>, std::vector<IndexList>>
    compute_vertices(const ProblemImpl& problem, Index begin_parameter_index, Index end_parameter_index, const LiteralList<StaticTag>& static_conditions);

    static std::tuple<IndexList, IndexList, IndexList>
    compute_edges(const ProblemImpl& problem, const LiteralList<StaticTag>& static_conditions, const Vertices& vertices);

    class EdgeIterator
    {
    private:
        const StaticConsistencyGraph* m_graph;
        size_t m_sources_pos;
        size_t m_targets_pos;

        const StaticConsistencyGraph& get_graph() const
        {
            assert(m_graph);
            return *m_graph;
        }

        void advance()
        {
            if (++m_targets_pos >= get_graph().m_target_offsets[m_sources_pos])
                ++m_sources_pos;
        }

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = Edge;
        using pointer = value_type*;
        using reference = const value_type&;
        using iterator_category = std::forward_iterator_tag;

        EdgeIterator() : m_graph(nullptr), m_sources_pos(0), m_targets_pos(0) {}
        EdgeIterator(const StaticConsistencyGraph& graph, bool begin) :
            m_graph(&graph),
            m_sources_pos(begin ? 0 : graph.m_sources.size()),
            m_targets_pos(begin ? 0 : graph.m_targets.size())
        {
        }
        value_type operator*() const
        {
            assert(m_sources_pos < get_graph().m_sources.size());
            assert(m_targets_pos < get_graph().m_targets.size());
            return Edge(get_graph().m_vertices[get_graph().m_sources[m_sources_pos]], get_graph().m_vertices[get_graph().m_targets[m_targets_pos]]);
        }
        EdgeIterator& operator++()
        {
            advance();
            return *this;
        }
        EdgeIterator operator++(int)
        {
            EdgeIterator tmp = *this;
            ++(*this);
            return tmp;
        }
        bool operator==(const EdgeIterator& other) const { return m_targets_pos == other.m_targets_pos && m_sources_pos == other.m_sources_pos; }
        bool operator!=(const EdgeIterator& other) const { return !(*this == other); }
    };

    friend class EdgeIterator;

    /// @brief Get an forward_range over immutable Vertices.
    auto get_vertices() const { return std::ranges::subrange(m_vertices.cbegin(), m_vertices.cend()); }

    auto get_edges() const { return std::ranges::subrange(EdgeIterator(*this, true), EdgeIterator(*this, false)); }

    size_t get_num_vertices() const { return m_vertices.size(); }
    size_t get_num_edges() const { return m_targets.size(); }

    /// @brief Get the vertex indices partitioned by the parameter index.
    const std::vector<IndexList>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

    /// @brief Get the object_index indices partitioned by the parameter index.
    const std::vector<IndexList>& get_objects_by_parameter_index() const { return m_objects_by_parameter_index; }

private:
    /* The data member of the consistency graph. */
    Vertices m_vertices;
    std::vector<IndexList> m_vertices_by_parameter_index;
    std::vector<IndexList> m_objects_by_parameter_index;

    // Adjacency list of edges.
    IndexList m_sources;  ///< sources with non-zero out-degree
    IndexList m_target_offsets;
    IndexList m_targets;
};

}

namespace mimir
{
/**
 * Print the graph nicely as dot format
 */
template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::StaticConsistencyGraph&, const formalism::ProblemImpl&>& data);
}

#endif
