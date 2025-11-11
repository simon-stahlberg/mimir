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

#ifndef MIMIR_FORMALISM_CONSISTENCY_GRAPH_HPP_
#define MIMIR_FORMALISM_CONSISTENCY_GRAPH_HPP_

#include "mimir/algorithms/shared_object_pool.hpp"
#include "mimir/common/closed_interval.hpp"
#include "mimir/common/declarations.hpp"
#include "mimir/common/formatter.hpp"
#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/problem_details.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <optional>
#include <ranges>
#include <sstream>
#include <vector>

namespace mimir::formalism
{

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

    /// @brief Get an forward_range over immutable Vertices.
    auto get_vertices() const { return std::ranges::subrange(m_vertices.cbegin(), m_vertices.cend()); }

    auto get_edges() const { return std::ranges::subrange(EdgeIterator(*this, true), EdgeIterator(*this, false)); }

public:
    StaticConsistencyGraph(const ProblemImpl& problem, ConjunctiveCondition condition, Index begin_parameter_index, Index end_parameter_index);

    /// @brief Useful to compute possible substitutions for conditional effects
    static std::tuple<Vertices, std::vector<IndexList>, std::vector<IndexList>>
    compute_vertices(const ProblemImpl& problem, ConjunctiveCondition condition, Index begin_parameter_index, Index end_parameter_index);

    static std::tuple<IndexList, IndexList, IndexList> compute_edges(const ProblemImpl& problem, ConjunctiveCondition condition, const Vertices& vertices);

    /// @brief Return an iterator over all vertices consistent with the precondition.
    /// @param static_details is the static assignment sets.
    /// @param dynamic_details is the dynamic assignment sets representing the given state.
    /// @return
    auto consistent_vertices(const StaticAssignmentSets& static_details,
                             const DynamicAssignmentSets& dynamic_details,
                             const std::optional<boost::dynamic_bitset<>>& vertex_mask) const
    {
        return get_vertices()
               | std::views::filter(
                   [this, &static_details, &dynamic_details, &vertex_mask](auto&& vertex)
                   {
                       return (vertex_mask.has_value() ? vertex_mask.value().test(vertex.get_index()) : true)
                              && vertex.consistent_literals(m_condition->get_literals<formalism::FluentTag>(), dynamic_details.fluent_predicate_assignment_sets)
                              && vertex.consistent_literals(m_condition->get_literals<formalism::DerivedTag>(),
                                                            dynamic_details.derived_predicate_assignment_sets)
                              && vertex.consistent_literals(m_condition->get_numeric_constraints(),
                                                            static_details.static_function_skeleton_assignment_sets,
                                                            dynamic_details.fluent_function_skeleton_assignment_sets);
                   });
    }

    /// @brief Return an iterator over all edges consistent with the precondition.
    /// @param static_details is the static assignment sets.
    /// @param dynamic_details is the dynamic assignment sets representing the given state.
    /// @return
    auto consistent_edges(const StaticAssignmentSets& static_details,
                          const DynamicAssignmentSets& dynamic_details,
                          const std::optional<boost::dynamic_bitset<>>& vertex_mask) const
    {
        auto vertex_mask_2 = get_shared_pool<boost::dynamic_bitset<>>().get_or_allocate();
        vertex_mask_2->resize(get_num_vertices());
        vertex_mask_2->reset();
        for (const auto& v : consistent_vertices(static_details, dynamic_details, vertex_mask))
            vertex_mask_2->set(v.get_index());

        return get_edges()
               | std::views::filter(
                   [this, mask = std::move(vertex_mask_2), &static_details, &dynamic_details](auto&& edge)
                   {
                       return mask->test(edge.get_src().get_index()) && mask->test(edge.get_dst().get_index())
                              && edge.consistent_literals(m_condition->get_literals<formalism::FluentTag>(), dynamic_details.fluent_predicate_assignment_sets)
                              && edge.consistent_literals(m_condition->get_literals<formalism::DerivedTag>(), dynamic_details.derived_predicate_assignment_sets)
                              && edge.consistent_literals(m_condition->get_numeric_constraints(),
                                                          static_details.static_function_skeleton_assignment_sets,
                                                          dynamic_details.fluent_function_skeleton_assignment_sets);
                   });
    }

    size_t get_num_vertices() const { return m_vertices.size(); }
    size_t get_num_edges() const { return m_targets.size(); }

    /// @brief Get the vertex indices partitioned by the parameter index.
    const std::vector<IndexList>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }
    const std::vector<IndexList>& get_objects_by_parameter_index() const { return m_objects_by_parameter_index; }

private:
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

private:
    ConjunctiveCondition m_condition;

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
std::ostream& print(std::ostream& out, const std::tuple<const formalism::StaticConsistencyGraph&, const formalism::ProblemImpl&>& data);
}

#endif
