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
        bool consistent_literals(const LiteralList<P>& literals, const AssignmentSet<P>& assignment_set) const;

        bool consistent_literals(const NumericConstraintList& numeric_constraints,
                                 const NumericAssignmentSet<StaticTag>& static_numeric_assignment_set,
                                 const NumericAssignmentSet<FluentTag>& fluent_numeric_assignment_set) const;

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
        bool consistent_literals(const LiteralList<P>& literals, const AssignmentSet<P>& assignment_set) const;

        bool consistent_literals(const NumericConstraintList& numeric_constraints,
                                 const NumericAssignmentSet<StaticTag>& static_numeric_assignment_set,
                                 const NumericAssignmentSet<FluentTag>& fluent_numeric_assignment_set) const;

        Index get_object_if_overlap(const Term& term) const;

        const Vertex& get_src() const { return m_src; }
        const Vertex& get_dst() const { return m_dst; }
    };

    using Vertices = std::vector<Vertex>;
    using Edges = std::vector<Edge>;

public:
    /// @brief Construct a static consistency graph
    /// @parameter_index problem The problem context.
    /// @parameter_index begin_parameter_index The first parameter index for which consistent assignments are represented.
    /// @parameter_index end_parameter_index The last parameter index plus one for which consistent assignments are represented.
    /// @parameter_index static_conditions The static literals for which a bindings must be found.
    /// @parameter_index static_assignment_set The assignment set of static initial literals.
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

    static Edges compute_edges(const ProblemImpl& problem, const LiteralList<StaticTag>& static_conditions, const Vertices& vertices);

    /// @brief Get the vertices.
    const Vertices& get_vertices() const { return m_vertices; }

    /// @brief Get the edges.
    const Edges& get_edges() const { return m_edges; }

    /// @brief Get the vertex indices partitioned by the parameter index.
    const std::vector<IndexList>& get_vertices_by_parameter_index() const { return m_vertices_by_parameter_index; }

    /// @brief Get the object_index indices partitioned by the parameter index.
    const std::vector<IndexList>& get_objects_by_parameter_index() const { return m_objects_by_parameter_index; }

private:
    /* The data member of the consistency graph. */
    Vertices m_vertices;
    std::vector<IndexList> m_vertices_by_parameter_index;
    std::vector<IndexList> m_objects_by_parameter_index;

    Edges m_edges;
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
