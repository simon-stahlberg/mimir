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

#include "mimir/formalism/consistency_graph.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

#include <cstdint>

namespace mimir::formalism
{
using Vertex = StaticConsistencyGraph::Vertex;
using Edge = StaticConsistencyGraph::Edge;
using Vertices = StaticConsistencyGraph::Vertices;

/**
 * VertexIndexIterator
 */

class VertexAssignmentIterator
{
private:
    const TermList* m_terms;
    const Vertex* m_vertex;
    Index m_pos;

    VertexAssignment m_assignment;

    const TermList& get_terms() const { return *m_terms; }
    const Vertex& get_vertex() const { return *m_vertex; }

    void advance()
    {
        /* Try to advance index. */
        for (Index index = m_assignment.index + 1; index < get_terms().size(); ++index)
        {
            auto object = get_vertex().get_object_if_overlap(get_terms()[index]);

            if (object != MAX_INDEX)
            {
                m_assignment.index = index;
                m_assignment.object = object;
                return;  ///< successfully generated vertex
            }
        }

        /* Failed to generate valid vertex assignment. */
        m_pos = MAX_INDEX;  ///< mark end of iteration
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = VertexAssignment;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    VertexAssignmentIterator() : m_terms(nullptr), m_vertex(nullptr), m_pos(MAX_INDEX) {}
    VertexAssignmentIterator(const TermList& terms, const Vertex& vertex, bool begin) : m_terms(&terms), m_vertex(&vertex), m_pos(begin ? 0 : MAX_INDEX)
    {
        if (begin)
        {
            advance();  // first advance might result in end immediately, e.g., if terms is empty.
        }
    }
    reference operator*() const { return m_assignment; }
    VertexAssignmentIterator& operator++()
    {
        advance();
        return *this;
    }
    VertexAssignmentIterator operator++(int)
    {
        VertexAssignmentIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const VertexAssignmentIterator& other) const { return m_pos == other.m_pos; }
    bool operator!=(const VertexAssignmentIterator& other) const { return !(*this == other); }
};

class VertexAssignmentRange
{
private:
    const TermList& m_terms;
    const Vertex& m_vertex;

public:
    VertexAssignmentRange(const TermList& terms, const Vertex& vertex) : m_terms(terms), m_vertex(vertex) {}

    VertexAssignmentIterator begin() const { return VertexAssignmentIterator(m_terms, m_vertex, true); }

    VertexAssignmentIterator end() const { return VertexAssignmentIterator(m_terms, m_vertex, false); }
};

/// @brief `EdgeAssignmentIterator` is used to generate vertices and edges in the consistency graph.
/// It is used in literals
///
/// It simultaneously iterates over vertices [x/o] and edges [x/o],[y/o'] with o < o'
/// to avoid having iterating over literals or numeric constraints twice.
class EdgeAssignmentIterator
{
private:
    const TermList* m_terms;
    const Edge* m_edge;
    Index m_pos;

    EdgeAssignment m_assignment;

    const TermList& get_terms() const { return *m_terms; }
    const Edge& get_edge() const { return *m_edge; }

    void advance()
    {
        if (m_assignment.second_index == MAX_INDEX)
        {
            /* Try to advance first_index. */

            // Reduced branching by setting iterator index and unsetting first index.
            // Note: unsetting first object is unnecessary because it will either be set or the iterator reaches its end.
            Index first_index = m_assignment.first_index + 1;
            m_assignment.first_index = MAX_INDEX;

            for (; first_index < get_terms().size(); ++first_index)
            {
                auto first_object = get_edge().get_object_if_overlap(get_terms()[first_index]);

                if (first_object != MAX_INDEX)
                {
                    m_assignment.first_index = first_index;
                    m_assignment.first_object = first_object;
                    m_assignment.second_index = first_index;
                    break;  ///< successfully generated left vertex
                }
            }
        }

        if (m_assignment.first_index != MAX_INDEX)
        {
            /* Try to advance second_index. */

            // Reduced branching by setting iterator index and unsetting second index and object
            Index second_index = m_assignment.second_index + 1;
            m_assignment.second_index = MAX_INDEX;
            m_assignment.second_object = MAX_INDEX;

            for (; second_index < get_terms().size(); ++second_index)
            {
                auto second_object = get_edge().get_object_if_overlap(get_terms()[second_index]);

                if (second_object != MAX_INDEX)
                {
                    m_assignment.second_index = second_index;
                    m_assignment.second_object = second_object;
                    return;  ///< successfully generated right vertex => successfully generated edge
                }
            }
        }

        if (m_assignment.second_object == MAX_INDEX)
        {
            /* Failed to generate valid edge assignment */

            m_pos = MAX_INDEX;  ///< mark end of iteration
        }
    }

public:
    using difference_type = std::ptrdiff_t;
    using value_type = EdgeAssignment;
    using pointer = value_type*;
    using reference = const value_type&;
    using iterator_category = std::forward_iterator_tag;

    EdgeAssignmentIterator() : m_terms(nullptr), m_edge(nullptr), m_pos(MAX_INDEX), m_assignment() {}
    EdgeAssignmentIterator(const TermList& terms, const Edge& edge, bool begin) : m_terms(&terms), m_edge(&edge), m_pos(begin ? 0 : MAX_INDEX), m_assignment()
    {
        if (begin)
        {
            advance();  // first advance might result in end immediately, e.g., if terms is empty.
        }
    }
    reference operator*() const { return m_assignment; }
    EdgeAssignmentIterator& operator++()
    {
        advance();
        return *this;
    }
    EdgeAssignmentIterator operator++(int)
    {
        EdgeAssignmentIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    bool operator==(const EdgeAssignmentIterator& other) const { return m_pos == other.m_pos; }
    bool operator!=(const EdgeAssignmentIterator& other) const { return !(*this == other); }
};

class EdgeAssignmentRange
{
private:
    const TermList& m_terms;
    const Edge& m_edge;

public:
    EdgeAssignmentRange(const TermList& terms, const Edge& edge) : m_terms(terms), m_edge(edge) {}

    EdgeAssignmentIterator begin() const { return EdgeAssignmentIterator(m_terms, m_edge, true); }

    EdgeAssignmentIterator end() const { return EdgeAssignmentIterator(m_terms, m_edge, false); }
};

template<typename T>
struct StructureTraits
{
};

template<>
struct StructureTraits<Vertex>
{
    using AssignmentIteratorType = VertexAssignmentIterator;
    using AssignmentRangeType = VertexAssignmentRange;
};

template<>
struct StructureTraits<Edge>
{
    using AssignmentIteratorType = EdgeAssignmentIterator;
    using AssignmentRangeType = EdgeAssignmentRange;
};

/**
 * consistent_literals_helper
 */

template<IsStaticOrFluentOrDerivedTag P>
static bool consistent_literals_helper(const LiteralList<P>& literals, const PredicateAssignmentSets<P>& predicate_assignment_sets, const Vertex& element)
{
    for (const auto& literal : literals)
    {
        const auto atom = literal->get_atom();
        const auto predicate = atom->get_predicate();
        const auto arity = predicate->get_arity();

        if (arity < 1)
        {
            continue;  ///< We test nullary literals separately
        }

        const auto negated = !literal->get_polarity();

        if (negated && arity != 1)
        {
            continue;  ///< Can only handly unary negated literals due to overapproximation
        }

        const auto& predicate_assignment_set = predicate_assignment_sets.get_set(predicate);
        const auto& terms = atom->get_terms();

        for (const auto& assignment : VertexAssignmentRange(terms, element))
        {
            assert(assignment.is_complete());

            const auto true_assignment = predicate_assignment_set[assignment];

            if (!negated && !true_assignment)
            {
                return false;
            }

            if (negated && true_assignment && (1 == arity))  ///< Due to overapproximation can only test valid assigned unary literals.
            {
                return false;
            }
        }
    }

    return true;
}

template<IsStaticOrFluentOrDerivedTag P>
static bool consistent_literals_helper(const LiteralList<P>& literals, const PredicateAssignmentSets<P>& predicate_assignment_sets, const Edge& element)
{
    for (const auto& literal : literals)
    {
        const auto atom = literal->get_atom();
        const auto predicate = atom->get_predicate();
        const auto arity = predicate->get_arity();

        if (arity < 2)
        {
            continue;  ///< We test nullary and unary literals separately.
        }

        const auto negated = !literal->get_polarity();

        if (negated && arity != 2)
        {
            continue;  ///< Can only handly binary negated literals due to overapproximation
        }

        const auto& predicate_assignment_set = predicate_assignment_sets.get_set(predicate);
        const auto& terms = atom->get_terms();

        /* Iterate edges. */

        for (const auto& assignment : EdgeAssignmentRange(terms, element))
        {
            assert(assignment.is_complete() && assignment.is_ordered());

            const auto true_assignment = predicate_assignment_set[assignment];

            if (!negated && !true_assignment)
            {
                return false;
            }

            if (negated && true_assignment && (2 == arity))  ///< Due to overapproximation can only test valid assigned binary literals.
            {
                return false;
            }
        }
    }

    return true;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
static Bounds<ContinuousCost>
compute_tighest_bound(Function<F> function, const Vertex& element, const FunctionSkeletonAssignmentSets<F>& function_skeleton_assignment_sets)
{
    const auto& function_skeleton_assignment_set = function_skeleton_assignment_sets.get_set(function->get_function_skeleton());
    const auto& terms = function->get_terms();

    // Compute tightest bound obtainable through an assignment of the edge substitutions
    auto bounds = function_skeleton_assignment_set[VertexAssignment()];

    for (const auto& assignment : VertexAssignmentRange(terms, element))
    {
        assert(assignment.is_complete());

        const auto assignment_bound = function_skeleton_assignment_set[assignment];

        bounds = Bounds<ContinuousCost>(std::max(bounds.get_lower(), assignment_bound.get_lower()), std::min(bounds.get_upper(), assignment_bound.get_upper()));
    }

    return bounds;
}

template<IsStaticOrFluentOrAuxiliaryTag F>
static Bounds<ContinuousCost>
compute_tighest_bound(Function<F> function, const Edge& element, const FunctionSkeletonAssignmentSets<F>& function_skeleton_assignment_sets)
{
    const auto& function_skeleton_assignment_set = function_skeleton_assignment_sets.get_set(function->get_function_skeleton());
    const auto& terms = function->get_terms();

    // Compute tightest bound obtainable through an assignment of the edge substitutions
    auto bounds = function_skeleton_assignment_set[VertexAssignment()];

    for (const auto& assignment : VertexAssignmentRange(terms, element.get_src()))
    {
        assert(assignment.is_complete());

        const auto assignment_bound = function_skeleton_assignment_set[assignment];

        bounds = Bounds<ContinuousCost>(std::max(bounds.get_lower(), assignment_bound.get_lower()), std::min(bounds.get_upper(), assignment_bound.get_upper()));
    }

    for (const auto& assignment : VertexAssignmentRange(terms, element.get_dst()))
    {
        assert(assignment.is_complete());

        const auto assignment_bound = function_skeleton_assignment_set[assignment];

        bounds = Bounds<ContinuousCost>(std::max(bounds.get_lower(), assignment_bound.get_lower()), std::min(bounds.get_upper(), assignment_bound.get_upper()));
    }

    for (const auto& assignment : EdgeAssignmentRange(terms, element))
    {
        assert(assignment.is_complete() && assignment.is_ordered());

        const auto assignment_bound = function_skeleton_assignment_set[assignment];

        bounds = Bounds<ContinuousCost>(std::max(bounds.get_lower(), assignment_bound.get_lower()), std::min(bounds.get_upper(), assignment_bound.get_upper()));
    }

    return bounds;
}

template<typename StructureType>
static Bounds<ContinuousCost> evaluate_function_expression_partially(FunctionExpression fexpr,
                                                                     const StructureType& element,
                                                                     const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                                                     const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets)
{
    return std::visit(
        [&](auto&& arg) -> Bounds<ContinuousCost>
        {
            using T = std::decay_t<decltype(arg)>;
            if constexpr (std::is_same_v<T, FunctionExpressionNumber>)
            {
                return Bounds<ContinuousCost>(arg->get_number(), arg->get_number());
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionBinaryOperator>)
            {
                return evaluate_binary_bounds(arg->get_binary_operator(),
                                              evaluate_function_expression_partially(arg->get_left_function_expression(),
                                                                                     element,
                                                                                     static_function_skeleton_assignment_sets,
                                                                                     fluent_function_skeleton_assignment_sets),
                                              evaluate_function_expression_partially(arg->get_right_function_expression(),
                                                                                     element,
                                                                                     static_function_skeleton_assignment_sets,
                                                                                     fluent_function_skeleton_assignment_sets));
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMultiOperator>)
            {
                auto left_values = std::vector<Bounds<ContinuousCost>> {};
                for (const auto& f : arg->get_function_expressions())
                {
                    left_values.push_back(
                        evaluate_function_expression_partially(f, element, static_function_skeleton_assignment_sets, fluent_function_skeleton_assignment_sets));
                }

                return std::accumulate(std::next(arg->get_function_expressions().begin()),  // Start from the second expression
                                       arg->get_function_expressions().end(),
                                       evaluate_function_expression_partially(arg->get_function_expressions().front(),
                                                                              element,
                                                                              static_function_skeleton_assignment_sets,
                                                                              fluent_function_skeleton_assignment_sets),
                                       [&](const auto& value, const auto& child_expr)
                                       {
                                           return evaluate_multi_bounds(arg->get_multi_operator(),
                                                                        value,
                                                                        evaluate_function_expression_partially(child_expr,
                                                                                                               element,
                                                                                                               static_function_skeleton_assignment_sets,
                                                                                                               fluent_function_skeleton_assignment_sets));
                                       });
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionMinus>)
            {
                const auto bounds = evaluate_function_expression_partially(arg->get_function_expression(),
                                                                           element,
                                                                           static_function_skeleton_assignment_sets,
                                                                           fluent_function_skeleton_assignment_sets);
                assert(-bounds.get_upper() < -bounds.get_lower());
                return Bounds<ContinuousCost>(-bounds.get_upper(), -bounds.get_lower());
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<StaticTag>>)
            {
                return compute_tighest_bound(arg->get_function(), element, static_function_skeleton_assignment_sets);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<FluentTag>>)
            {
                return compute_tighest_bound(arg->get_function(), element, fluent_function_skeleton_assignment_sets);
            }
            else if constexpr (std::is_same_v<T, FunctionExpressionFunction<AuxiliaryTag>>)
            {
                throw std::runtime_error("computing_remapping(fexpr, term_to_position, remapping): Unexpected FunctionExpression type.");
            }
            else
            {
                static_assert(dependent_false<T>::value, "collect_terms_helper(fexpr, ref_terms): Missing implementation for FunctionExpression type.");
            }
        },
        fexpr->get_variant());
}

template<typename StructureType>
static bool is_partially_evaluated_constraint_satisfied(NumericConstraint numeric_constraint,
                                                        const StructureType& element,
                                                        const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                                        const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets)
{
    const auto lhs = evaluate_function_expression_partially(numeric_constraint->get_left_function_expression(),
                                                            element,
                                                            static_function_skeleton_assignment_sets,
                                                            fluent_function_skeleton_assignment_sets);

    const auto rhs = evaluate_function_expression_partially(numeric_constraint->get_right_function_expression(),
                                                            element,
                                                            static_function_skeleton_assignment_sets,
                                                            fluent_function_skeleton_assignment_sets);

    return evaluate(numeric_constraint->get_binary_comparator(), lhs, rhs);
}

static bool consistent_numeric_constraints_helper(const NumericConstraintList& numeric_constraints,
                                                  const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                                  const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets,
                                                  const Vertex& element)
{
    for (const auto& numeric_constraint : numeric_constraints)
    {
        const auto& terms = numeric_constraint->get_terms();
        const auto& arity = terms.size();

        if (arity < 1)
        {
            continue;  ///< We test nullary constraints separately.
        }

        if (!is_partially_evaluated_constraint_satisfied(numeric_constraint,
                                                         element,
                                                         static_function_skeleton_assignment_sets,
                                                         fluent_function_skeleton_assignment_sets))
        {
            return false;
        }
    }

    return true;
}

static bool consistent_numeric_constraints_helper(const NumericConstraintList& numeric_constraints,
                                                  const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                                  const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets,
                                                  const Edge& element)
{
    for (const auto& numeric_constraint : numeric_constraints)
    {
        const auto& terms = numeric_constraint->get_terms();
        const auto& arity = terms.size();

        if (arity < 2)
        {
            continue;  ///< We test nullary and unary constraints separately.
        }

        if (!is_partially_evaluated_constraint_satisfied(numeric_constraint,
                                                         element,
                                                         static_function_skeleton_assignment_sets,
                                                         fluent_function_skeleton_assignment_sets))
        {
            return false;
        }
    }

    return true;
}

/**
 * Vertex
 */

template<IsStaticOrFluentOrDerivedTag P>
bool Vertex::consistent_literals(const LiteralList<P>& literals, const PredicateAssignmentSets<P>& predicate_assignment_sets) const
{
    return consistent_literals_helper<P>(literals, predicate_assignment_sets, *this);
}

template bool Vertex::consistent_literals(const LiteralList<StaticTag>& literals, const PredicateAssignmentSets<StaticTag>& predicate_assignment_sets) const;
template bool Vertex::consistent_literals(const LiteralList<FluentTag>& literals, const PredicateAssignmentSets<FluentTag>& predicate_assignment_sets) const;
template bool Vertex::consistent_literals(const LiteralList<DerivedTag>& literals, const PredicateAssignmentSets<DerivedTag>& predicate_assignment_sets) const;

bool Vertex::consistent_literals(const NumericConstraintList& numeric_constraints,
                                 const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                                 const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets) const
{
    return consistent_numeric_constraints_helper(numeric_constraints,
                                                 static_function_skeleton_assignment_sets,
                                                 fluent_function_skeleton_assignment_sets,
                                                 *this);
}

Index Vertex::get_object_if_overlap(const Term& term) const
{
    if (const auto object = std::get_if<Object>(&term->get_variant()))
    {
        return (*object)->get_index();
    }

    if (const auto variable = std::get_if<Variable>(&term->get_variant()))
    {
        if (m_parameter_index == (*variable)->get_parameter_index())
        {
            return m_object_index;
        }
    }

    return MAX_INDEX;
}

/**
 * Edge
 */

template<IsStaticOrFluentOrDerivedTag P>
bool Edge::consistent_literals(const LiteralList<P>& literals, const PredicateAssignmentSets<P>& predicate_assignment_sets) const
{
    return consistent_literals_helper<P>(literals, predicate_assignment_sets, *this);
}

template bool Edge::consistent_literals(const LiteralList<StaticTag>& literals, const PredicateAssignmentSets<StaticTag>& predicate_assignment_sets) const;
template bool Edge::consistent_literals(const LiteralList<FluentTag>& literals, const PredicateAssignmentSets<FluentTag>& predicate_assignment_sets) const;
template bool Edge::consistent_literals(const LiteralList<DerivedTag>& literals, const PredicateAssignmentSets<DerivedTag>& predicate_assignment_sets) const;

bool Edge::consistent_literals(const NumericConstraintList& numeric_constraints,
                               const FunctionSkeletonAssignmentSets<StaticTag>& static_function_skeleton_assignment_sets,
                               const FunctionSkeletonAssignmentSets<FluentTag>& fluent_function_skeleton_assignment_sets) const
{
    return consistent_numeric_constraints_helper(numeric_constraints,
                                                 static_function_skeleton_assignment_sets,
                                                 fluent_function_skeleton_assignment_sets,
                                                 *this);
}

Index Edge::get_object_if_overlap(const Term& term) const
{
    // This code is faster than std::visit.

    if (const auto object = std::get_if<Object>(&term->get_variant()))
    {
        return (*object)->get_index();
    }

    if (const auto variable = std::get_if<Variable>(&term->get_variant()))
    {
        const auto parameter_index = (*variable)->get_parameter_index();

        if (m_src.get_parameter_index() == parameter_index)
        {
            return m_src.get_object_index();
        }
        else if (m_dst.get_parameter_index() == parameter_index)
        {
            return m_dst.get_object_index();
        }
    }

    return MAX_INDEX;
}

/**
 * StaticConsistencyGraph
 */

std::tuple<Vertices, std::vector<IndexList>, std::vector<IndexList>> StaticConsistencyGraph::compute_vertices(const ProblemImpl& problem,
                                                                                                              Index begin_parameter_index,
                                                                                                              Index end_parameter_index,
                                                                                                              const LiteralList<StaticTag>& static_conditions)
{
    const auto& static_predicate_assignment_sets = problem.get_positive_static_initial_predicate_assignment_sets();

    auto vertices = Vertices {};
    auto vertices_by_parameter_index = std::vector<IndexList> {};
    auto objects_by_parameter_index = std::vector<IndexList> {};

    for (Index parameter_index = begin_parameter_index; parameter_index < end_parameter_index; ++parameter_index)
    {
        IndexList vertex_partition;
        IndexList object_partition;

        for (const auto& object : problem.get_problem_and_domain_objects())
        {
            const auto object_index = object->get_index();
            const auto vertex_index = static_cast<Index>(vertices.size());

            auto vertex = Vertex(vertex_index, parameter_index, object_index);

            if (vertex.consistent_literals(static_conditions, static_predicate_assignment_sets))
            {
                vertex_partition.push_back(vertex_index);
                object_partition.push_back(object_index);
                vertices.push_back(std::move(vertex));
            }
        }
        vertices_by_parameter_index.push_back(std::move(vertex_partition));
        objects_by_parameter_index.push_back(std::move(object_partition));
    }

    return std::make_tuple(std::move(vertices), std::move(vertices_by_parameter_index), std::move(objects_by_parameter_index));
}

std::tuple<IndexList, IndexList, IndexList>
StaticConsistencyGraph::compute_edges(const ProblemImpl& problem, const LiteralList<StaticTag>& static_conditions, const Vertices& vertices)
{
    const auto& static_predicate_assignment_sets = problem.get_positive_static_initial_predicate_assignment_sets();

    auto sources = IndexList {};

    auto target_offsets = IndexList {};
    target_offsets.reserve(vertices.size());

    auto targets = IndexList {};

    for (Index first_vertex_index = 0; first_vertex_index < vertices.size(); ++first_vertex_index)
    {
        const auto targets_before = targets.size();
        for (Index second_vertex_index = (first_vertex_index + 1); second_vertex_index < vertices.size(); ++second_vertex_index)
        {
            const auto& first_vertex = vertices.at(first_vertex_index);
            const auto& second_vertex = vertices.at(second_vertex_index);

            assert(first_vertex.get_index() == first_vertex_index);
            assert(second_vertex.get_index() == second_vertex_index);

            // Part 1 of definition of substitution consistency graph (Stahlberg-ecai2023): exclude I^\neq
            if (first_vertex.get_parameter_index() != second_vertex.get_parameter_index()
                && Edge(first_vertex, second_vertex).consistent_literals(static_conditions, static_predicate_assignment_sets))
            {
                targets.push_back(second_vertex_index);
            }
        }

        if (targets_before < targets.size())
        {
            sources.push_back(first_vertex_index);
            target_offsets.push_back(targets.size());
        }
    }

    return { std::move(sources), std::move(target_offsets), std::move(targets) };
}

StaticConsistencyGraph::StaticConsistencyGraph(const ProblemImpl& problem,
                                               Index begin_parameter_index,
                                               Index end_parameter_index,
                                               const LiteralList<StaticTag>& static_conditions)
{
    auto [vertices_, vertices_by_parameter_index_, objects_by_parameter_index_] =
        compute_vertices(problem, begin_parameter_index, end_parameter_index, static_conditions);

    m_vertices = std::move(vertices_);
    m_vertices_by_parameter_index = std::move(vertices_by_parameter_index_);
    m_objects_by_parameter_index = std::move(objects_by_parameter_index_);

    auto [sources_, target_offsets_, targets_] = compute_edges(problem, static_conditions, m_vertices);

    m_sources = std::move(sources_);
    m_target_offsets = std::move(target_offsets_);
    m_targets = std::move(targets_);
}

}

namespace mimir
{
template<>
std::ostream& operator<<(std::ostream& out, const std::tuple<const formalism::StaticConsistencyGraph&, const formalism::ProblemImpl&>& data)
{
    const auto& [graph, problem] = data;

    const auto create_node = [](const formalism::Vertex& vertex, const formalism::ProblemImpl& problem, std::ostream& out)
    {
        out << "  \"" << vertex.get_index() << "\" [label=\"#" << vertex.get_parameter_index() << " <- "
            << problem.get_repositories().get_object(vertex.get_object_index()) << "\"];\n";
    };

    const auto create_edge = [](const formalism::Edge& edge, std::ostream& out)
    { out << "  \"" << edge.get_src().get_index() << "\" -- \"" << edge.get_dst().get_index() << "\";\n"; };

    // Define the undirected graph
    out << "graph myGraph {\n";
    // Define the nodes
    for (const auto& vertex : graph.get_vertices())
    {
        create_node(vertex, problem, out);
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