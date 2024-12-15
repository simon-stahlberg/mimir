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

#include "mimir/search/satisficing_binding_generator/consistency_graph.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"

#include <cstdint>

namespace mimir::consistency_graph
{
// To print vectors as [e1,...,en]
using mimir::operator<<;

/**
 * VertexIndexIterator
 */

class VertexAssignmentIterator
{
private:
    // We use this as special value and when adding 1 we obtain 0.
    static const Index UNDEFINED = std::numeric_limits<Index>::max();

    const TermList& m_terms;
    const Vertex& m_vertex;

    Index m_index;
    Index m_object;

    Index get_object_if_overlap(const Term& term)
    {
        if (const auto object = std::get_if<Object>(&term->get_variant()))
        {
            return (*object)->get_index();
        }

        if (const auto variable = std::get_if<Variable>(&term->get_variant()))
        {
            if (m_vertex.get_parameter_index() == (*variable)->get_parameter_index())
            {
                return m_vertex.get_object_index();
            }
        }

        return UNDEFINED;
    }

    void find_next_binding()
    {
        auto found = false;

        for (size_t index = m_index + 1; index < m_terms.size(); ++index)
        {
            assert(index < m_terms.size());

            auto object = get_object_if_overlap(m_terms[index]);

            if (object != UNDEFINED)
            {
                m_index = index;
                m_object = object;
                found = true;
                break;
            }
        }

        if (!found)
        {
            m_index = UNDEFINED;
            m_object = UNDEFINED;
        }
    }

public:
    VertexAssignmentIterator(const TermList& terms, const Vertex& vertex) : m_terms(terms), m_vertex(vertex), m_index(UNDEFINED), m_object(UNDEFINED)
    {
        find_next_binding();
    }

    bool has_next() const { return m_index < m_terms.size(); }

    Assignment next()
    {
        if (!has_next())
        {
            throw std::out_of_range("No more bindings available");
        }

        auto binding = Assignment(m_index, m_object, UNDEFINED, UNDEFINED);
        find_next_binding();
        return binding;
    }
};

/**
 * EdgeAssignmentIterator
 */

class EdgeAssignmentIterator
{
private:
    // We use this as special value and when adding 1 we obtain 0.
    static const Index UNDEFINED = std::numeric_limits<Index>::max();

    const TermList& m_terms;
    const Edge& m_edge;

    Index m_first_index;
    Index m_second_index;
    Index m_first_object;
    Index m_second_object;

    Index get_object_if_overlap(const Term& term)
    {
        if (const auto object = std::get_if<Object>(&term->get_variant()))
        {
            return (*object)->get_index();
        }

        if (const auto variable = std::get_if<Variable>(&term->get_variant()))
        {
            const auto parameter_index = (*variable)->get_parameter_index();

            if (m_edge.get_src().get_parameter_index() == parameter_index)
            {
                return m_edge.get_src().get_object_index();
            }
            else if (m_edge.get_dst().get_parameter_index() == parameter_index)
            {
                return m_edge.get_dst().get_object_index();
            }
        }

        return UNDEFINED;
    }

    void find_next_binding()
    {
        if (m_second_index == UNDEFINED)
        {
            auto found_first = false;

            for (size_t first_index = m_first_index + 1; first_index < m_terms.size(); ++first_index)
            {
                assert(first_index < m_terms.size());

                auto first_object = get_object_if_overlap(m_terms[first_index]);

                if (first_object != UNDEFINED)
                {
                    m_first_index = first_index;
                    m_first_object = first_object;
                    m_second_index = first_index;
                    found_first = true;
                    break;
                }
            }

            if (!found_first)
            {
                m_first_index = UNDEFINED;
                m_first_object = UNDEFINED;
                m_second_index = UNDEFINED;
                m_second_object = UNDEFINED;
            }
        }

        if (m_first_index != UNDEFINED)
        {
            auto found_second = false;

            for (size_t second_index = m_second_index + 1; second_index < m_terms.size(); ++second_index)
            {
                assert(second_index < m_terms.size());

                auto second_object = get_object_if_overlap(m_terms[second_index]);

                if (second_object != UNDEFINED)
                {
                    m_second_index = second_index;
                    m_second_object = second_object;
                    found_second = true;
                    break;
                }
            }

            if (!found_second)
            {
                m_second_index = UNDEFINED;
                m_second_object = UNDEFINED;
            }
        }
    }

public:
    EdgeAssignmentIterator(const TermList& terms, const Edge& edge) :
        m_terms(terms),
        m_edge(edge),
        m_first_index(UNDEFINED),
        m_second_index(UNDEFINED),
        m_first_object(UNDEFINED),
        m_second_object(UNDEFINED)
    {
        find_next_binding();
    }

    bool has_next() const { return m_first_index < m_terms.size(); }

    Assignment next()
    {
        if (!has_next())
        {
            throw std::out_of_range("No more bindings available");
        }

        auto binding = Assignment(m_first_index, m_first_object, m_second_index, m_second_object);
        find_next_binding();
        return binding;
    }
};

/**
 * consistent_literals_helper
 */

template<typename T>
concept IsVertexOrEdge = std::is_same_v<T, Vertex> || std::is_same_v<T, Edge>;

template<IsVertexOrEdge T>
struct ElementTypeTraits
{
};

template<>
struct ElementTypeTraits<Vertex>
{
    using IteratorType = VertexAssignmentIterator;
};

template<>
struct ElementTypeTraits<Edge>
{
    using IteratorType = EdgeAssignmentIterator;
};

template<PredicateTag P, IsVertexOrEdge ElementType>
bool consistent_literals_helper(const LiteralList<P>& literals, const AssignmentSet<P>& assignment_set, const ElementType& element)
{
    using IteratorType = typename ElementTypeTraits<ElementType>::IteratorType;

    const auto num_objects = assignment_set.get_num_objects();
    const auto& per_predicate_assignment_set = assignment_set.get_per_predicate_assignment_set();

    for (const auto& literal : literals)
    {
        const auto arity = literal->get_atom()->get_predicate()->get_arity();
        const auto negated = literal->is_negated();

        if (negated && arity != 1 && arity != 2)
        {
            continue;
        }

        const auto& predicate_assignment_set = per_predicate_assignment_set[literal->get_atom()->get_predicate()->get_index()];
        const auto& terms = literal->get_atom()->get_terms();
        auto assignment_iterator = IteratorType(terms, element);

        while (assignment_iterator.has_next())
        {
            const auto assignment = assignment_iterator.next();
            assert(assignment.first_index < arity);
            assert(assignment.first_index < assignment.second_index);

            const auto assignment_rank = get_assignment_rank(assignment, arity, num_objects);
            const auto true_assignment = predicate_assignment_set[assignment_rank];

            if (!negated && !true_assignment)
            {
                return false;
            }

            if (negated && true_assignment && (assignment.size() == arity))
            {
                return false;
            }
        }
    }

    return true;
}

/**
 * Vertex
 */

template<PredicateTag P>
bool Vertex::consistent_literals(const LiteralList<P>& literals, const AssignmentSet<P>& assignment_set) const
{
    return consistent_literals_helper<P>(literals, assignment_set, *this);
}

template bool Vertex::consistent_literals(const LiteralList<Static>& literals, const AssignmentSet<Static>& assignment_set) const;
template bool Vertex::consistent_literals(const LiteralList<Fluent>& literals, const AssignmentSet<Fluent>& assignment_set) const;
template bool Vertex::consistent_literals(const LiteralList<Derived>& literals, const AssignmentSet<Derived>& assignment_set) const;

/**
 * Edge
 */

template<PredicateTag P>
bool Edge::consistent_literals(const LiteralList<P>& literals, const AssignmentSet<P>& assignment_set) const
{
    return consistent_literals_helper<P>(literals, assignment_set, *this);
}

template bool Edge::consistent_literals(const LiteralList<Static>& literals, const AssignmentSet<Static>& assignment_set) const;
template bool Edge::consistent_literals(const LiteralList<Fluent>& literals, const AssignmentSet<Fluent>& assignment_set) const;
template bool Edge::consistent_literals(const LiteralList<Derived>& literals, const AssignmentSet<Derived>& assignment_set) const;

/**
 * StaticConsistencyGraph
 */

StaticConsistencyGraph::StaticConsistencyGraph(Problem problem,
                                               size_t begin_parameter_index,
                                               size_t end_parameter_index,
                                               const LiteralList<Static>& static_conditions) :
    m_problem(problem)
{
    const auto& static_assignment_set = m_problem->get_static_assignment_set();

    /* 1. Compute vertices */

    for (uint32_t parameter_index = begin_parameter_index; parameter_index < end_parameter_index; ++parameter_index)
    {
        VertexIndexList vertex_partition;
        ObjectIndexList object_partition;

        for (const auto& object : m_problem->get_objects())
        {
            const auto object_index = object->get_index();
            const auto vertex_index = VertexIndex { m_vertices.size() };
            auto vertex = Vertex(vertex_index, parameter_index, object_index);

            if (vertex.consistent_literals(static_conditions, static_assignment_set))
            {
                vertex_partition.push_back(vertex_index);
                object_partition.push_back(object_index);
                m_vertices.push_back(std::move(vertex));
            }
        }
        m_vertices_by_parameter_index.push_back(std::move(vertex_partition));
        m_objects_by_parameter_index.push_back(std::move(object_partition));
    }

    /* 2. Compute edges */

    for (size_t first_vertex_index = 0; first_vertex_index < m_vertices.size(); ++first_vertex_index)
    {
        for (size_t second_vertex_index = (first_vertex_index + 1); second_vertex_index < m_vertices.size(); ++second_vertex_index)
        {
            const auto& first_vertex = m_vertices.at(first_vertex_index);
            const auto& second_vertex = m_vertices.at(second_vertex_index);

            // Part 1 of definition of substitution consistency graph (Stahlberg-ecai2023): exclude I^\neq
            if (first_vertex.get_parameter_index() != second_vertex.get_parameter_index())
            {
                auto edge = Edge(Vertex(first_vertex_index, first_vertex.get_parameter_index(), first_vertex.get_object_index()),
                                 Vertex(second_vertex_index, second_vertex.get_parameter_index(), second_vertex.get_object_index()));

                if (edge.consistent_literals(static_conditions, static_assignment_set))
                {
                    m_edges.push_back(std::move(edge));
                }
            }
        }
    }
}

}

namespace mimir
{
template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const consistency_graph::StaticConsistencyGraph&, const PDDLRepositories&>& data)
{
    const auto& [graph, pddl_repositories] = data;

    const auto create_node = [](const consistency_graph::Vertex& vertex, const PDDLRepositories& pddl_repositories, std::ostream& out)
    {
        out << "  \"" << vertex.get_index() << "\" [label=\"#" << vertex.get_parameter_index() << " <- "
            << *pddl_repositories.get_object(vertex.get_object_index()) << "\"];\n";
    };

    const auto create_edge = [](const consistency_graph::Edge& edge, std::ostream& out)
    { out << "  \"" << edge.get_src().get_index() << "\" -- \"" << edge.get_dst().get_index() << "\";\n"; };

    // Define the undirected graph
    out << "graph myGraph {\n";
    // Define the nodes
    for (const auto& vertex : graph.get_vertices())
    {
        create_node(vertex, pddl_repositories, out);
    }
    // Define the edges
    for (const auto& edge : graph.get_edges())
    {
        create_edge(edge, out);
    }
    out << "}\n";
    return out;
}

}
