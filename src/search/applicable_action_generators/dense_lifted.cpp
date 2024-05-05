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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/dense_lifted.hpp"

#include "mimir/algorithms/kpkc.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/search/actions/dense.hpp"

#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <stdexcept>

using namespace std::string_literals;

namespace mimir
{
namespace consistency_graph
{
// To print vectors as [e1,...,en]
using mimir::operator<<;

StaticConsistencyGraph::StaticConsistencyGraph(Problem problem, Action action, const PDDLFactories& factories) : m_problem(problem)
{
    /* 1. Compute vertices */

    auto static_initial_atoms = GroundAtomList {};
    to_ground_atoms(m_problem->get_static_initial_literals(), static_initial_atoms);
    const auto static_assignment_set = consistency_graph::AssignmentSet(m_problem, static_initial_atoms);

    // Bookkeeping to test whether [x/o] satisfies static preconditions.
    // We use this data structure to avoid explicit grounding.
    auto unary_groundings = std::unordered_map<Predicate, std::unordered_set<Object>> {};
    for (const auto& literal : problem->get_static_initial_literals())
    {
        const auto& atom = literal->get_atom();
        const auto& predicate = atom->get_predicate();
        if (predicate->get_arity() == 1)
        {
            const auto& object = atom->get_objects().front();

            unary_groundings[predicate].insert(object);
        }
    }

    for (uint32_t parameter_index = 0; parameter_index < action->get_arity(); ++parameter_index)
    {
        VertexIDs partition;

        for (const auto& object : m_problem->get_objects())
        {
            // Check whether [x/o] satisfies static preconditions
            // Adding this test results in a smaller consistency graph
            // and resulted in 20% performance increase on the tests.
            bool all_static_unary_preconditions_satisfied = true;
            for (const auto& literal : action->get_static_conditions())
            {
                if (literal->get_atom()->get_predicate()->get_arity() == 1)
                {
                    if (const auto term_variable = std::get_if<TermVariableImpl>(literal->get_atom()->get_terms().front()))
                    {
                        if (term_variable->get_variable()->get_parameter_index() == parameter_index)
                        {
                            // Check whether the unary ground atom is true in the initial state
                            if (!unary_groundings[literal->get_atom()->get_predicate()].count(object))
                            {
                                all_static_unary_preconditions_satisfied = false;
                            }
                        }
                    }
                }
            }
            if (all_static_unary_preconditions_satisfied)
            {
                // D: Partition [x/o] by x
                auto vertex_id = VertexID { m_vertices.size() };
                partition.push_back(vertex_id);
                // D: Create nodes [x/o]
                m_vertices.push_back(Vertex(vertex_id, parameter_index, object->get_identifier()));
            }
        }
        m_vertices_by_parameter_index.push_back(std::move(partition));
    }

    /* 2. Compute edges */

    for (size_t first_id = 0; first_id < m_vertices.size(); ++first_id)
    {
        for (size_t second_id = (first_id + 1); second_id < m_vertices.size(); ++second_id)
        {
            const auto& first_vertex = m_vertices.at(first_id);
            const auto& second_vertex = m_vertices.at(second_id);

            if (first_vertex.get_param_index() != second_vertex.get_param_index())
            {
                const auto edge =
                    consistency_graph::Edge(consistency_graph::Vertex(first_id, first_vertex.get_param_index(), first_vertex.get_object_index()),
                                            consistency_graph::Vertex(second_id, second_vertex.get_param_index(), second_vertex.get_object_index()));

                if (static_assignment_set.literal_all_consistent(action->get_static_conditions(), edge))
                {
                    m_edges.push_back(edge);
                }
            }
        }
    }
}

std::ostream& operator<<(std::ostream& out, const StaticConsistencyGraph& graph)
{
    out << "StaticConsistencyGraph(\n"
        << "    vertices=" << graph.get_vertices() << "\n"
        << "    edges=" << graph.get_edges() << std::endl;
    return out;
}

static size_t
get_assignment_position(int32_t first_position, int32_t first_object, int32_t second_position, int32_t second_object, int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto rank = (first * (first_position + 1)) + (second * (second_position + 1)) + (third * (first_object + 1)) + (fourth * (second_object + 1));
    return (size_t) rank;
}

static size_t num_assignments(int32_t arity, int32_t num_objects)
{
    const auto first = 1;
    const auto second = first * (arity + 1);
    const auto third = second * (arity + 1);
    const auto fourth = third * (num_objects + 1);
    const auto max = (first * arity) + (second * arity) + (third * num_objects) + (fourth * num_objects);
    return (size_t) (max + 1);
}

AssignmentSet::AssignmentSet(Problem problem, const GroundAtomList& ground_atoms) : m_problem(problem)
{
    const auto num_objects = problem->get_objects().size();
    const auto& predicates = problem->get_domain()->get_predicates();

    // D: Create assignment set for each predicate
    m_f.resize(predicates.size());
    for (const auto& predicate : predicates)
    {
        // Predicates must have indexing 0,1,2,... for this implementation
        assert(predicate->get_identifier() < static_cast<int>(predicates.size()));

        auto& assignment_set = m_f[predicate->get_identifier()];
        assignment_set.resize(num_assignments(predicate->get_arity(), num_objects));
    }
    // D: Initialize m_f
    for (const auto& ground_atom : ground_atoms)
    {
        const auto& arity = ground_atom->get_arity();
        const auto& predicate = ground_atom->get_predicate();
        const auto& arguments = ground_atom->get_objects();
        auto& assignment_set = m_f[predicate->get_identifier()];

        for (size_t first_position = 0; first_position < arity; ++first_position)
        {
            const auto& first_object = arguments[first_position];
            assignment_set[get_assignment_position(first_position, first_object->get_identifier(), -1, -1, arity, num_objects)] = true;

            for (size_t second_position = first_position + 1; second_position < arity; ++second_position)
            {
                const auto& second_object = arguments[second_position];
                assignment_set[get_assignment_position(second_position, second_object->get_identifier(), -1, -1, arity, num_objects)] = true;
                assignment_set[get_assignment_position(first_position,
                                                       first_object->get_identifier(),
                                                       second_position,
                                                       second_object->get_identifier(),
                                                       arity,
                                                       num_objects)] = true;
            }
        }
    }
}

bool AssignmentSet::literal_all_consistent(const std::vector<Literal>& literals, const Edge& consistent_edge) const
{
    for (const auto& literal : literals)
    {
        int32_t first_position = -1;
        int32_t second_position = -1;
        int32_t first_object_id = -1;
        int32_t second_object_id = -1;
        bool empty_assignment = true;

        const auto arity = literal->get_atom()->get_predicate()->get_arity();

        // TODO: decide whether we want to split this into unary and binary to make the distinction clear.
        for (std::size_t index = 0; index < literal->get_atom()->get_predicate()->get_arity(); ++index)
        {
            const auto& term = literal->get_atom()->get_terms()[index];

            if (const auto term_object = std::get_if<TermObjectImpl>(term))
            {
                if (arity <= 2)
                {
                    const auto object_id = term_object->get_object()->get_identifier();

                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(object_id);
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(object_id);
                    }

                    empty_assignment = false;
                }
            }
            else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
            {
                const auto parameter_index = term_variable->get_variable()->get_parameter_index();

                // D: [parameter_index]
                if (consistent_edge.get_src().get_param_index() == parameter_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(consistent_edge.get_src().get_object_index());
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(consistent_edge.get_src().get_object_index());
                        break;
                    }

                    empty_assignment = false;
                }
                else if (consistent_edge.get_dst().get_param_index() == parameter_index)
                {
                    if (first_position < 0)
                    {
                        first_position = index;
                        first_object_id = static_cast<int32_t>(consistent_edge.get_dst().get_object_index());
                    }
                    else
                    {
                        second_position = index;
                        second_object_id = static_cast<int32_t>(consistent_edge.get_dst().get_object_index());
                        break;
                    }

                    empty_assignment = false;
                }
            }
        }

        if (!empty_assignment)
        {
            const auto& assignment_set = m_f[literal->get_atom()->get_predicate()->get_identifier()];
            const auto assignment_rank = get_assignment_position(first_position,
                                                                 first_object_id,
                                                                 second_position,
                                                                 second_object_id,
                                                                 static_cast<int32_t>(arity),
                                                                 static_cast<int32_t>(m_problem->get_objects().size()));
            const auto consistent_with_state = assignment_set[assignment_rank];

            if (!literal->is_negated() && !consistent_with_state)
            {
                return false;
            }
            else if (literal->is_negated() && consistent_with_state && ((arity == 1) || ((arity == 2) && (second_position >= 0))))
            {
                return false;
            }
        }
    }

    return true;
}
}

/// @brief Grounding function for pddl structures.
static void ground_variables(const TermList& terms, const ObjectList& binding, ObjectList& out_terms)
{
    out_terms.clear();

    for (const auto& term : terms)
    {
        if (const auto term_object = std::get_if<TermObjectImpl>(term))
        {
            out_terms.emplace_back(term_object->get_object());
        }
        else if (const auto term_variable = std::get_if<TermVariableImpl>(term))
        {
            out_terms.emplace_back(binding[term_variable->get_variable()->get_parameter_index()]);
        }
    }
}

GroundLiteral AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_literal(const Literal& literal, const ObjectList& binding) const
{
    ObjectList grounded_terms;
    ground_variables(literal->get_atom()->get_terms(), binding, grounded_terms);
    auto grounded_atom = m_pddl_factories.get_or_create_ground_atom(literal->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = m_pddl_factories.get_or_create_ground_literal(literal->is_negated(), grounded_atom);
    return grounded_literal;
}

class GroundAndEvaluateFunctionExpressionVisitor
{
private:
    const GroundFunctionToValue& m_ground_function_value_costs;
    const ObjectList& m_binding;
    PDDLFactories& m_pddl_factories;

    GroundFunction ground_function(const Function& function)
    {
        auto grounded_terms = ObjectList {};
        ground_variables(function->get_terms(), m_binding, grounded_terms);
        return m_pddl_factories.get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);
    }

public:
    GroundAndEvaluateFunctionExpressionVisitor(const GroundFunctionToValue& ground_function_value_costs,
                                               const ObjectList& binding,
                                               PDDLFactories& pddl_factories) :

        m_ground_function_value_costs(ground_function_value_costs),
        m_binding(binding),
        m_pddl_factories(pddl_factories)
    {
    }

    double operator()(const FunctionExpressionNumberImpl& expr) { return expr.get_number(); }

    double operator()(const FunctionExpressionBinaryOperatorImpl& expr)
    {
        return evaluate_binary(expr.get_binary_operator(),
                               std::visit(*this, *expr.get_left_function_expression()),
                               std::visit(*this, *expr.get_right_function_expression()));
    }

    double operator()(const FunctionExpressionMultiOperatorImpl& expr)
    {
        assert(!expr.get_function_expressions().empty());

        auto it = expr.get_function_expressions().begin();
        auto result = std::visit(*this, **it);
        for (; it != expr.get_function_expressions().end(); ++it)
        {
            result = evaluate_multi(expr.get_multi_operator(), result, std::visit(*this, **it));
        }

        return result;
    }

    double operator()(const FunctionExpressionMinusImpl& expr) { return -std::visit(*this, *expr.get_function_expression()); }

    double operator()(const FunctionExpressionFunctionImpl& expr)
    {
        auto grounded_function = ground_function(expr.get_function());

        auto it = m_ground_function_value_costs.find(grounded_function);
        if (it == m_ground_function_value_costs.end())
        {
            throw std::runtime_error("No numeric fluent available to determine cost for ground function "s + grounded_function->str());
        }
        const auto cost = it->second;

        return cost;
    }
};

ConstView<ActionDispatcher<DenseStateTag>> AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_action(const Action& action, ObjectList&& binding)
{
    const auto fill_bitsets =
        [this, &binding](const std::vector<Literal>& literals, flat::BitsetBuilder& ref_positive_bitset, flat::BitsetBuilder& ref_negative_bitset)
    {
        for (const auto& literal : literals)
        {
            const auto grounded_literal = ground_literal(literal, binding);

            if (grounded_literal->is_negated())
            {
                ref_negative_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
            else
            {
                ref_positive_bitset.set(grounded_literal->get_atom()->get_identifier());
            }
        }
    };

    auto& positive_precondition = m_action_builder.get_applicability_positive_precondition_bitset();
    auto& negative_precondition = m_action_builder.get_applicability_negative_precondition_bitset();

    positive_precondition.unset_all();
    negative_precondition.unset_all();

    fill_bitsets(action->get_static_conditions(), positive_precondition, negative_precondition);
    fill_bitsets(action->get_fluent_conditions(), positive_precondition, negative_precondition);

    auto& positive_effect = m_action_builder.get_unconditional_positive_effect_bitset();
    auto& negative_effect = m_action_builder.get_unconditional_negative_effect_bitset();

    positive_effect.unset_all();
    negative_effect.unset_all();

    // Handle simple effects
    auto effect_literals = LiteralList {};
    for (const auto& effect : action->get_simple_effects())
    {
        effect_literals.push_back(effect->get_effect());
    }
    fill_bitsets(effect_literals, positive_effect, negative_effect);
    // TODO: Handle conditional effects
    if (!action->get_conditional_effects().empty())
    {
        throw std::runtime_error("Conditional effects are not implemented.");
    }
    // TODO: Handle universal effects
    if (!action->get_universal_effects().empty())
    {
        throw std::runtime_error("Universal effects are not implemented.");
    }

    m_action_builder.get_id() = m_actions.size();
    m_action_builder.get_cost() = std::visit(GroundAndEvaluateFunctionExpressionVisitor(m_ground_function_value_costs, binding, this->m_pddl_factories),
                                             *action->get_function_expression());
    m_action_builder.get_action() = action;
    auto& objects = m_action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj);
    }

    auto& flatmemory_builder = m_action_builder.get_flatmemory_builder();
    flatmemory_builder.finish();

    const auto [iter, inserted] = m_actions.insert(flatmemory_builder);
    const auto result_action = DenseAction(*iter);
    if (inserted)
    {
        m_actions_by_index.push_back(result_action);
    }

    return result_action;
}

/// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
bool AAG<LiftedAAGDispatcher<DenseStateTag>>::nullary_preconditions_hold(const Action& action, DenseState state) const
{
    for (const auto& literal : action->get_fluent_conditions())
    {
        if (literal->get_atom()->get_predicate()->get_arity() == 0 && !state.literal_holds(ground_literal(literal, {})))
        {
            return false;
        }
    }

    return true;
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::nullary_case(const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.

    const auto grounded_action = ground_action(action, {});

    if (grounded_action.is_applicable(state))
    {
        out_applicable_actions.emplace_back(grounded_action);
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::unary_case(const Action& action, DenseState state, std::vector<DenseAction>& out_applicable_actions)
{
    // There is only one parameter, try all bindings with the correct type.

    for (const auto& object : m_problem->get_objects())
    {
        auto grounded_action = ground_action(action, { object });

        if (grounded_action.is_applicable(state))
        {
            out_applicable_actions.emplace_back(grounded_action);
        }
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::general_case(const consistency_graph::AssignmentSet& assignment_sets,
                                                           const Action& action,
                                                           DenseState state,
                                                           std::vector<DenseAction>& out_applicable_actions)
{
    const auto& consistency_graph = m_static_consistency_graphs.at(action);
    const auto num_vertices = consistency_graph.get_vertices().size();

    std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

    // D: Restrict statically consistent assignments based on the assignments in the current state
    //    and build the consistency graph as an adjacency matrix
    for (const auto& edge : consistency_graph.get_edges())
    {
        if (assignment_sets.literal_all_consistent(action->get_fluent_conditions(), edge))
        {
            const auto first_id = edge.get_src().get_id();
            const auto second_id = edge.get_dst().get_id();
            auto& first_row = adjacency_matrix[first_id];
            auto& second_row = adjacency_matrix[second_id];
            first_row[second_id] = 1;
            second_row[first_id] = 1;
        }
    }

    // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
    // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
    // parameters) tends to be very small.

    const auto& partitions = consistency_graph.get_vertices_by_parameter_index();
    std::vector<std::vector<std::size_t>> cliques;
    find_all_k_cliques_in_k_partite_graph(adjacency_matrix, partitions, cliques);

    for (const auto& clique : cliques)
    {
        auto terms = ObjectList(action->get_arity());

        for (std::size_t vertex_index = 0; vertex_index < action->get_arity(); ++vertex_index)
        {
            const auto vertex_id = clique[vertex_index];
            const auto& vertex = consistency_graph.get_vertices()[vertex_id];
            const auto parameter_index = vertex.get_param_index();
            const auto object_id = vertex.get_object_index();
            terms[parameter_index] = m_pddl_factories.get_object(object_id);
        }

        const auto grounded_action = ground_action(action, std::move(terms));

        // TODO: We do not need to check applicability if action consists of at most binary predicates in the precondition.
        // Add this information to the FlatAction struct.

        if (grounded_action.is_applicable(state))
        {
            out_applicable_actions.push_back(grounded_action);
        }
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::generate_applicable_actions_impl(DenseState state, std::vector<DenseAction>& out_applicable_actions)
{
    out_applicable_actions.clear();

    // Create the assignment sets that are shared by all action schemas.

    auto ground_atoms = GroundAtomList {};
    for (const auto& atom_id : state.get_atoms_bitset())
    {
        ground_atoms.push_back(m_pddl_factories.get_ground_atom(atom_id));
    }

    const auto assignment_sets = consistency_graph::AssignmentSet(m_problem, ground_atoms);

    // Get the applicable ground actions for each action schema.

    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        if (nullary_preconditions_hold(action, state))
        {
            if (action->get_arity() == 0)
            {
                nullary_case(action, state, out_applicable_actions);
            }
            else if (action->get_arity() == 1)
            {
                unary_case(action, state, out_applicable_actions);
            }
            else
            {
                general_case(assignment_sets, action, state, out_applicable_actions);
            }
        }
    }
}

AAG<LiftedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) :
    m_problem(problem),
    m_pddl_factories(pddl_factories),
    m_ground_function_value_costs(),
    m_static_consistency_graphs()
{
    // 1. Error checking
    for (const auto& literal : problem->get_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("negative literals in the initial state is not supported");
        }
    }

    // 2. Initialize ground function costs
    for (const auto numeric_fluent : problem->get_numeric_fluents())
    {
        m_ground_function_value_costs.emplace(numeric_fluent->get_function(), numeric_fluent->get_number());
    }

    // 3. Initialize static consistency graph

    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        // The following is used only by the general case.

        if (action->get_arity() >= 2)
        {
            bool skip = false;
            for (const auto& literal : action->get_static_conditions())
            {
                if (literal->get_atom()->get_predicate()->get_arity() == 0)
                {
                    const auto negated = literal->is_negated();
                    const auto& atom = literal->get_atom();
                    auto contains = false;

                    for (const auto& initial_literal : problem->get_initial_literals())
                    {
                        if (atom->get_predicate() == initial_literal->get_atom()->get_predicate())
                        {
                            contains = true;
                            break;
                        }
                    }

                    if (contains == negated)
                    {
                        skip = true;
                        break;
                    }
                }
            }
            if (skip)
            {
                continue;
            }

            m_static_consistency_graphs.emplace(action, consistency_graph::StaticConsistencyGraph(m_problem, action, m_pddl_factories));
        }
    }
}

[[nodiscard]] const flat::DenseActionSet& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_actions() const { return m_actions; }

[[nodiscard]] ConstView<ActionDispatcher<DenseStateTag>> AAG<LiftedAAGDispatcher<DenseStateTag>>::get_action(size_t action_id) const
{
    return m_actions_by_index.at(action_id);
}

}
