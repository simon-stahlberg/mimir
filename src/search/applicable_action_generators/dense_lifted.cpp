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
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/search/actions/dense.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/grounding_utils.hpp"

#include <boost/dynamic_bitset.hpp>
#include <iostream>
#include <stdexcept>

using namespace std::string_literals;

namespace mimir
{

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

const std::vector<AxiomPartition>& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_axiom_partitioning() const
{
    return m_axiom_evaluator.get_axiom_partitioning();
}

GroundAxiom AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_axiom(const Axiom& axiom, ObjectList&& binding)
{
    return m_axiom_evaluator.ground_axiom(axiom, std::move(binding));
}

ConstView<ActionDispatcher<DenseStateTag>> AAG<LiftedAAGDispatcher<DenseStateTag>>::ground_action(const Action& action, ObjectList&& binding)
{
    const auto fill_bitsets =
        [this](const std::vector<Literal>& literals, FlatBitsetBuilder& ref_positive_bitset, FlatBitsetBuilder& ref_negative_bitset, const auto& binding)
    {
        for (const auto& literal : literals)
        {
            const auto grounded_literal = ground_literal(literal, binding, m_pddl_factories);

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

    const auto fill_effect = [this](const Literal& literal, FlatSimpleEffect& ref_effect, const auto& binding)
    {
        const auto grounded_literal = ground_literal(literal, binding, m_pddl_factories);
        ref_effect.is_negated = grounded_literal->is_negated();
        ref_effect.atom_id = grounded_literal->get_atom()->get_identifier();
    };

    /* Header */

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

    /* Precondition */
    auto& positive_precondition = m_action_builder.get_applicability_positive_precondition_bitset();
    auto& negative_precondition = m_action_builder.get_applicability_negative_precondition_bitset();
    positive_precondition.unset_all();
    negative_precondition.unset_all();
    fill_bitsets(action->get_conditions(), positive_precondition, negative_precondition, binding);

    /* Simple effects */
    auto& positive_effect = m_action_builder.get_unconditional_positive_effect_bitset();
    auto& negative_effect = m_action_builder.get_unconditional_negative_effect_bitset();
    positive_effect.unset_all();
    negative_effect.unset_all();
    auto effect_literals = LiteralList {};
    for (const auto& effect : action->get_simple_effects())
    {
        effect_literals.push_back(effect->get_effect());
    }
    fill_bitsets(effect_literals, positive_effect, negative_effect, binding);

    /* Conditional effects */
    // Fetch data
    auto& positive_conditional_preconditions = m_action_builder.get_conditional_positive_precondition_bitsets();
    auto& negative_conditional_preconditions = m_action_builder.get_conditional_negative_precondition_bitsets();
    auto& conditional_effects = m_action_builder.get_conditional_effects();

    // Resize builders.
    // TODO: this might cause reallocation, we probably want to set "actual" size to 0 and keep its size.
    const auto num_conditional_effects = action->get_conditional_effects().size();
    positive_conditional_preconditions.resize(num_conditional_effects);
    negative_conditional_preconditions.resize(num_conditional_effects);
    conditional_effects.resize(num_conditional_effects);
    if (num_conditional_effects > 0)
    {
        for (size_t i = 0; i < num_conditional_effects; ++i)
        {
            positive_conditional_preconditions[i].unset_all();
            negative_conditional_preconditions[i].unset_all();

            // Ground conditions and effect
            fill_bitsets(action->get_conditional_effects().at(i)->get_conditions(),
                         positive_conditional_preconditions[i],
                         negative_conditional_preconditions[i],
                         binding);

            fill_effect(action->get_conditional_effects().at(i)->get_effect(), conditional_effects[i], binding);
        }
    }

    /* Universal effects */
    const auto num_universal_effects = action->get_universal_effects().size();
    if (num_universal_effects > 0)
    {
        const auto& graphs = m_static_consistency_graphs.at(action);
        const auto binding_size = binding.size();
        for (size_t i = 0; i < num_universal_effects; ++i)
        {
            // Fetch data
            const auto& universal_effect = action->get_universal_effects().at(i);
            const auto& consistency_graph = graphs.get_universal_effect_graphs().at(i);
            const auto& objects_by_parameter_index = consistency_graph.get_objects_by_parameter_index();

            // Resize builders.
            const auto num_conditional_effects = Combinations(objects_by_parameter_index).num_combinations();
            const auto old_size = positive_conditional_preconditions.size();
            positive_conditional_preconditions.resize(old_size + num_conditional_effects);
            negative_conditional_preconditions.resize(old_size + num_conditional_effects);
            conditional_effects.resize(old_size + num_conditional_effects);

            // Create binding and ground conditions and effect
            binding.resize(binding_size + universal_effect->get_arity());

            // The position to place the conditional precondition + effect
            auto j = old_size;
            assert(!objects_by_parameter_index.empty());
            for (const auto& combination : Combinations(objects_by_parameter_index))
            {
                // Create binding
                for (size_t pos = 0; pos < universal_effect->get_arity(); ++pos)
                {
                    const auto object_id = *combination[pos];
                    binding[binding_size + pos] = m_pddl_factories.get_object(object_id);
                }

                // Ground conditions and effect
                positive_conditional_preconditions[j].unset_all();
                negative_conditional_preconditions[j].unset_all();

                fill_bitsets(universal_effect->get_conditions(), positive_conditional_preconditions[j], negative_conditional_preconditions[j], binding);
                fill_effect(universal_effect->get_effect(), conditional_effects[j], binding);

                ++j;
            }
        }
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
        if (literal->get_atom()->get_predicate()->get_arity() == 0 && !state.literal_holds(ground_literal(literal, {}, m_pddl_factories)))
        {
            return false;
        }
    }

    return true;
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::nullary_case(const Action& action, DenseState state, DenseActionList& out_applicable_actions)
{
    // There are no parameters, meaning that the preconditions are already fully ground. Simply check if the single ground action is applicable.

    const auto grounded_action = ground_action(action, {});

    // TODO: this also unnecessarily grounds universal effects
    if (grounded_action.is_applicable(state))
    {
        m_applicable_actions.insert(grounded_action);
        out_applicable_actions.emplace_back(grounded_action);
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::unary_case(const Action& action, DenseState state, DenseActionList& out_applicable_actions)
{
    // There is only one parameter, try all bindings with the correct type.

    for (const auto& object : m_problem->get_objects())
    {
        auto grounded_action = ground_action(action, { object });

        // TODO: this also unnecessarily grounds universal effects
        if (grounded_action.is_applicable(state))
        {
            m_applicable_actions.insert(grounded_action);
            out_applicable_actions.emplace_back(grounded_action);
        }
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::general_case(const AssignmentSet& assignment_sets,
                                                           const Action& action,
                                                           DenseState state,
                                                           DenseActionList& out_applicable_actions)
{
    const auto& graphs = m_static_consistency_graphs.at(action);
    assert(graphs.get_precondition_graph().has_value());
    const auto& precondition_graph = graphs.get_precondition_graph().value();
    const auto num_vertices = precondition_graph.get_vertices().size();

    std::vector<boost::dynamic_bitset<>> adjacency_matrix(num_vertices, boost::dynamic_bitset<>(num_vertices));

    // D: Restrict statically consistent assignments based on the assignments in the current state
    //    and build the consistency graph as an adjacency matrix
    for (const auto& edge : precondition_graph.get_edges())
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

    const auto& partitions = precondition_graph.get_vertices_by_parameter_index();
    std::vector<std::vector<std::size_t>> cliques;
    find_all_k_cliques_in_k_partite_graph(adjacency_matrix, partitions, cliques);

    for (const auto& clique : cliques)
    {
        auto terms = ObjectList(action->get_arity());

        for (std::size_t vertex_index = 0; vertex_index < action->get_arity(); ++vertex_index)
        {
            const auto vertex_id = clique[vertex_index];
            const auto& vertex = precondition_graph.get_vertices()[vertex_id];
            const auto parameter_index = vertex.get_param_index();
            const auto object_id = vertex.get_object_index();
            terms[parameter_index] = m_pddl_factories.get_object(object_id);
        }

        const auto grounded_action = ground_action(action, std::move(terms));

        // TODO: We do not need to check applicability if action consists of at most binary predicates in the precondition.
        // Add this information to the FlatAction struct.

        // TODO: this also unnecessarily grounds universal effects
        if (grounded_action.is_applicable(state))
        {
            m_applicable_actions.insert(grounded_action);
            out_applicable_actions.push_back(grounded_action);
        }
    }
}

void AAG<LiftedAAGDispatcher<DenseStateTag>>::generate_applicable_actions_impl(DenseState state, DenseActionList& out_applicable_actions)
{
    out_applicable_actions.clear();

    // Create the assignment sets that are shared by all action schemas.

    auto ground_atoms = GroundAtomList {};
    for (const auto& atom_id : state.get_atoms_bitset())
    {
        ground_atoms.push_back(m_pddl_factories.get_ground_atom(atom_id));
    }

    const auto assignment_sets = AssignmentSet(m_problem, ground_atoms);

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

void AAG<LiftedAAGDispatcher<DenseStateTag>>::generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_state_atoms, FlatBitsetBuilder& ref_derived_atoms)
{
    // In the lifted case, we use the axiom evaluator.
    m_axiom_evaluator.generate_and_apply_axioms(ref_state_atoms, ref_derived_atoms);
}

AAG<LiftedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) :
    m_problem(problem),
    m_pddl_factories(pddl_factories),
    m_axiom_evaluator(problem, pddl_factories),
    m_ground_function_value_costs(),
    m_static_consistency_graphs()
{
    /* 1. Error checking */

    for (const auto& literal : problem->get_initial_literals())
    {
        if (literal->is_negated())
        {
            throw std::runtime_error("Negative literals in the initial state is not supported.");
        }
    }

    /* 2. Initialize ground function costs */

    for (const auto numeric_fluent : problem->get_numeric_fluents())
    {
        m_ground_function_value_costs.emplace(numeric_fluent->get_function(), numeric_fluent->get_number());
    }

    /* 3. Initialize static consistency graph */

    auto static_initial_atoms = GroundAtomList {};
    to_ground_atoms(m_problem->get_static_initial_literals(), static_initial_atoms);
    const auto static_assignment_set = AssignmentSet(m_problem, static_initial_atoms);

    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        m_static_consistency_graphs.emplace(action, consistency_graph::Graphs(m_problem, action, static_assignment_set));
    }
}

const DenseAxiomSet& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_applicable_axioms() const { return m_axiom_evaluator.get_applicable_axioms(); }

const FlatDenseAxiomSet& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_axioms() const { return m_axiom_evaluator.get_axioms(); }

const DenseActionSet& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_applicable_actions() const { return m_applicable_actions; }

const FlatDenseActionSet& AAG<LiftedAAGDispatcher<DenseStateTag>>::get_actions() const { return m_actions; }

ConstView<ActionDispatcher<DenseStateTag>> AAG<LiftedAAGDispatcher<DenseStateTag>>::get_action(size_t action_id) const
{
    return m_actions_by_index.at(action_id);
}

}
