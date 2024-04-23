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
#include "mimir/search/actions/dense.hpp"

#include <boost/dynamic_bitset.hpp>
#include <stdexcept>

using namespace std::string_literals;

namespace mimir
{
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

/// @brief Grounding function for flat structures.
void AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_variables(const std::vector<ParameterIndexOrConstantId>& variables,
                                                               const ObjectList& binding,
                                                               ObjectList& out_terms) const
{
    out_terms.clear();

    for (const auto& variable : variables)
    {
        if (variable.is_constant())
        {
            out_terms.emplace_back(m_pddl_factories.get_object(variable.get_value()));
        }
        else
        {
            out_terms.emplace_back(binding[variable.get_value()]);
        }
    }
}

GroundLiteral AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_literal(const FlatLiteral& literal, const ObjectList& binding) const
{
    ObjectList grounded_terms;
    ground_variables(literal.arguments, binding, grounded_terms);
    auto grounded_atom = m_pddl_factories.get_or_create_ground_atom(literal.source->get_atom()->get_predicate(), grounded_terms);
    auto grounded_literal = m_pddl_factories.get_or_create_ground_literal(literal.source->is_negated(), grounded_atom);
    return grounded_literal;
}

class GroundAndEvaluateFunctionExpressionVisitor
{
private:
    const std::map<GroundFunction, double>& m_initial_ground_function_values;
    const ObjectList& m_binding;
    PDDLFactories& m_pddl_factories;

    GroundFunction ground_function(const Function& function)
    {
        auto grounded_terms = ObjectList {};
        ground_variables(function->get_terms(), m_binding, grounded_terms);
        return m_pddl_factories.get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);
    }

public:
    GroundAndEvaluateFunctionExpressionVisitor(const std::map<GroundFunction, double>& initial_ground_function_values,
                                               const ObjectList& binding,
                                               PDDLFactories& pddl_factories) :

        m_initial_ground_function_values(initial_ground_function_values),
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

        auto it = m_initial_ground_function_values.find(grounded_function);
        if (it == m_initial_ground_function_values.end())
        {
            throw std::runtime_error("No numeric fluent available to determine cost for ground function "s + grounded_function->str());
        }
        const auto cost = it->second;

        return cost;
    }
};

ConstView<ActionDispatcher<DenseStateTag>> AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_action(const FlatAction& flat_action, ObjectList&& binding)
{
    const auto fill_bitsets = [this, &binding](const std::vector<FlatLiteral>& literals, BitsetBuilder& ref_positive_bitset, BitsetBuilder& ref_negative_bitset)
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

    fill_bitsets(flat_action.static_precondition, positive_precondition, negative_precondition);
    fill_bitsets(flat_action.fluent_precondition, positive_precondition, negative_precondition);

    auto& positive_effect = m_action_builder.get_unconditional_positive_effect_bitset();
    auto& negative_effect = m_action_builder.get_unconditional_negative_effect_bitset();

    positive_effect.unset_all();
    negative_effect.unset_all();

    fill_bitsets(flat_action.unconditional_effect, positive_effect, negative_effect);

    m_action_builder.get_id() = m_actions.size();
    m_action_builder.get_cost() = std::visit(GroundAndEvaluateFunctionExpressionVisitor(m_initial_ground_function_values, binding, this->m_pddl_factories),
                                             *flat_action.source->get_function_expression());
    m_action_builder.get_action() = flat_action.source;
    auto& objects = m_action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj);
    }

    auto& flatmemory_builder = m_action_builder.get_flatmemory_builder();
    flatmemory_builder.finish();

    const auto [iter, inserted] = m_actions.insert(flatmemory_builder);
    const auto result_action = ConstActionView(*iter);
    if (inserted)
    {
        m_actions_by_index.push_back(result_action);
    }

    return result_action;
}

/// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
bool AAG<LiftedAAGDispatcher<DenseStateTag>>::nullary_preconditions_hold(const FlatAction& flat_action, ConstStateView state) const
{
    for (const auto& literal : flat_action.fluent_precondition)
    {
        if (literal.arity == 0 && state.literal_holds(ground_literal(literal, {})))
        {
            return false;
        }
    }

    return true;
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::nullary_case(const FlatAction& flat_action,
                                                           ConstStateView state,
                                                           std::vector<ConstActionView>& out_applicable_actions)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.

    const auto grounded_action = ground_action(flat_action, {});

    if (grounded_action.is_applicable(state))
    {
        out_applicable_actions.emplace_back(grounded_action);
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::unary_case(const FlatAction& flat_action,
                                                         ConstStateView state,
                                                         std::vector<ConstActionView>& out_applicable_actions)
{
    // There is only one parameter, try all bindings with the correct type.

    for (const auto& object : m_problem->get_objects())
    {
        auto grounded_action = ground_action(flat_action, { object });

        if (grounded_action.is_applicable(state))
        {
            out_applicable_actions.emplace_back(grounded_action);
        }
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::general_case(const std::vector<std::vector<bool>>& assignment_sets,
                                                           const FlatAction& flat_action,
                                                           ConstStateView state,
                                                           std::vector<ConstActionView>& out_applicable_actions)
{
    const auto& to_vertex_assignment = m_to_vertex_assignment.at(flat_action.source);
    const auto& statically_consistent_assignments = m_statically_consistent_assignments.at(flat_action.source);
    const auto num_vertices = to_vertex_assignment.size();

    std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

    for (const auto& assignment : statically_consistent_assignments)
    {
        const auto& first_assignment = assignment.first_assignment;
        const auto& second_assignment = assignment.second_assignment;

        if (literal_all_consistent(assignment_sets, flat_action.fluent_precondition, first_assignment, second_assignment, m_problem))
        {
            const auto first_id = assignment.first_position;
            const auto second_id = assignment.second_position;
            auto& first_row = adjacency_matrix[first_id];
            auto& second_row = adjacency_matrix[second_id];
            first_row[second_id] = 1;
            second_row[first_id] = 1;
        }
    }

    // Find all cliques of size num_parameters whose labels denote complete assignments that might yield an applicable precondition. The relatively few
    // atoms in the state (compared to the number of possible atoms) lead to very sparse graphs, so the number of maximal cliques of maximum size (#
    // parameters) tends to be very small.

    const auto& partitions = m_partitions.at(flat_action.source);
    std::vector<std::vector<std::size_t>> cliques;
    find_all_k_cliques_in_k_partite_graph(adjacency_matrix, partitions, cliques);

    for (const auto& clique : cliques)
    {
        auto terms = ObjectList(flat_action.arity);

        for (std::size_t vertex_index = 0; vertex_index < flat_action.arity; ++vertex_index)
        {
            const auto vertex_id = clique[vertex_index];
            const auto& vertex_assignment = to_vertex_assignment.at(vertex_id);
            const auto parameter_index = vertex_assignment.parameter_index;
            const auto object_id = vertex_assignment.object_id;
            terms[parameter_index] = m_pddl_factories.get_object(object_id);
        }

        const auto grounded_action = ground_action(flat_action, std::move(terms));

        // TODO: We do not need to check applicability if action consists of at most binary predicates in the precondition.
        // Add this information to the FlatAction struct.

        if (grounded_action.is_applicable(state))
        {
            out_applicable_actions.push_back(grounded_action);
        }
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::generate_applicable_actions_impl(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions)
{
    out_applicable_actions.clear();

    // Create the assignment sets that are shared by all action schemas.

    std::vector<size_t> atom_ids;

    for (const auto& atom_id : state.get_atoms_bitset())
    {
        atom_ids.emplace_back(atom_id);
    }

    const auto assignment_sets = build_assignment_sets(m_problem, atom_ids, m_pddl_factories);

    // Get the applicable ground actions for each action schema.

    for (const auto& flat_action : m_flat_actions)
    {
        if (nullary_preconditions_hold(flat_action, state))
        {
            if (flat_action.arity == 0)
            {
                nullary_case(flat_action, state, out_applicable_actions);
            }
            else if (flat_action.arity == 1)
            {
                unary_case(flat_action, state, out_applicable_actions);
            }
            else
            {
                general_case(assignment_sets, flat_action, state, out_applicable_actions);
            }
        }
    }
}

AAG<LiftedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) :
    m_problem(problem),
    m_initial_ground_function_values(),
    m_pddl_factories(pddl_factories),
    m_flat_actions(),
    m_partitions(),
    m_to_vertex_assignment(),
    m_statically_consistent_assignments()
{
    for (const auto numeric_fluent : problem->get_numeric_fluents())
    {
        m_initial_ground_function_values.emplace(numeric_fluent->get_function(), numeric_fluent->get_number());
    }

    const auto& domain = problem->get_domain();

    for (const auto& action : domain->get_actions())
    {
        m_flat_actions.emplace_back(FlatAction(domain, action));
    }

    for (const auto& flat_action : m_flat_actions)
    {
        std::vector<std::vector<size_t>> partitions;
        std::vector<Assignment> to_vertex_assignment;
        std::vector<AssignmentPair> statically_consistent_assignments;

        // The following is used only by the general case.

        if (flat_action.arity >= 2)
        {
            // Create a mapping between indices and parameter-object mappings.

            for (uint32_t parameter_index = 0; parameter_index < flat_action.arity; ++parameter_index)
            {
                std::vector<std::size_t> partition;

                for (const auto& object : m_problem->get_objects())
                {
                    partition.push_back(to_vertex_assignment.size());
                    to_vertex_assignment.push_back(Assignment(parameter_index, object->get_identifier()));
                }

                partitions.push_back(std::move(partition));
            }

            // Filter assignment based on static atoms.

            const auto& static_predicates = problem->get_domain()->get_static_predicates();
            std::vector<size_t> static_atom_ids;

            for (const auto& literal : problem->get_initial_literals())
            {
                if (literal->is_negated())
                {
                    throw std::runtime_error("negative literals in the initial state is not supported");
                }

                const auto& atom = literal->get_atom();

                if (std::count(static_predicates.begin(), static_predicates.end(), atom->get_predicate()))
                {
                    static_atom_ids.emplace_back(atom->get_identifier());
                }
            }

            const auto assignment_sets = build_assignment_sets(problem, static_atom_ids, pddl_factories);

            for (size_t first_id = 0; first_id < to_vertex_assignment.size(); ++first_id)
            {
                for (size_t second_id = (first_id + 1); second_id < to_vertex_assignment.size(); ++second_id)
                {
                    const auto& first_assignment = to_vertex_assignment.at(first_id);
                    const auto& second_assignment = to_vertex_assignment.at(second_id);

                    if (first_assignment.parameter_index != second_assignment.parameter_index)
                    {
                        if (literal_all_consistent(assignment_sets, flat_action.static_precondition, first_assignment, second_assignment, problem))
                        {
                            statically_consistent_assignments.push_back(AssignmentPair(first_id, first_assignment, second_id, second_assignment));
                        }
                    }
                }
            }

            for (const auto& literal : flat_action.static_precondition)
            {
                if (literal.arity == 0)
                {
                    const auto negated = literal.source->is_negated();
                    const auto& atom = literal.source->get_atom();
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
                        statically_consistent_assignments.clear();
                        break;
                    }
                }
            }
        }

        m_partitions.emplace(flat_action.source, std::move(partitions));
        m_to_vertex_assignment.emplace(flat_action.source, std::move(to_vertex_assignment));
        m_statically_consistent_assignments.emplace(flat_action.source, std::move(statically_consistent_assignments));
    }
}

[[nodiscard]] const DenseActionSet& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_actions() const { return m_actions; }

[[nodiscard]] ConstView<ActionDispatcher<DenseStateTag>> AAG<LiftedAAGDispatcher<DenseStateTag>>::get_action(size_t action_id) const
{
    if (action_id < m_actions.size())
    {
        return m_actions_by_index[action_id];
    }

    throw std::invalid_argument("invalid identifier");
}

}
