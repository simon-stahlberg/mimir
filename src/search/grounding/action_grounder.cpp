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

#include "mimir/search/grounding/action_grounder.hpp"

#include "mimir/common/itertools.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/action.hpp"

namespace mimir
{

class GroundAndEvaluateFunctionExpressionVisitor
{
private:
    const GroundFunctionToValue& m_ground_function_to_cost;
    const ObjectList& m_binding;
    PDDLRepositories& m_pddl_repositories;

    GroundFunction ground_function(const Function& function)
    {
        auto grounded_terms = ObjectList {};
        m_pddl_repositories.ground_variables(function->get_terms(), m_binding, grounded_terms);
        return m_pddl_repositories.get_or_create_ground_function(function->get_function_skeleton(), grounded_terms);
    }

public:
    GroundAndEvaluateFunctionExpressionVisitor(const GroundFunctionToValue& ground_function_value_cost,
                                               const ObjectList& binding,
                                               PDDLRepositories& ref_pddl_repositories) :

        m_ground_function_to_cost(ground_function_value_cost),
        m_binding(binding),
        m_pddl_repositories(ref_pddl_repositories)
    {
    }

    double operator()(const FunctionExpressionImpl& expr)
    {
        return std::visit([this](auto&& arg) -> double { return (*this)(*arg); }, expr.get_variant());
    }

    double operator()(const FunctionExpressionNumberImpl& expr) { return expr.get_number(); }

    double operator()(const FunctionExpressionBinaryOperatorImpl& expr)
    {
        return evaluate_binary(expr.get_binary_operator(), (*this)(*expr.get_left_function_expression()), (*this)(*expr.get_right_function_expression()));
    }

    double operator()(const FunctionExpressionMultiOperatorImpl& expr)
    {
        auto result = ContinuousCost(0);
        for (const auto& child_expr : expr.get_function_expressions())
        {
            result = evaluate_multi(expr.get_multi_operator(), result, (*this)(*child_expr));
        }

        return result;
    }

    double operator()(const FunctionExpressionMinusImpl& expr) { return -(*this)(*expr.get_function_expression()); }

    double operator()(const FunctionExpressionFunctionImpl& expr)
    {
        auto grounded_function = ground_function(expr.get_function());

        auto it = m_ground_function_to_cost.find(grounded_function);
        if (it == m_ground_function_to_cost.end())
        {
            throw std::runtime_error("No numeric fluent available to determine cost for ground function " + to_string(*grounded_function));
        }
        const auto cost = it->second;

        return cost;
    }
};

/// @brief Simplest construction
ActionGrounder::ActionGrounder(Problem problem, std::shared_ptr<PDDLRepositories> pddl_repositories) :
    m_problem(problem),
    m_pddl_repositories(std::move(pddl_repositories)),
    m_action_precondition_grounders(),
    m_action_conditional_effects(),
    m_actions(),
    m_actions_by_index(),
    m_action_builder(),
    m_action_groundings(),
    m_ground_function_to_cost()
{
    /* 1. Initialize ground function costs. */

    for (const auto numeric_fluent : problem->get_numeric_fluents())
    {
        m_ground_function_to_cost.emplace(numeric_fluent->get_function(), numeric_fluent->get_number());
    }

    /* 2. Initialize the condition grounders for each action schema. */

    auto static_initial_atoms = to_ground_atoms(m_problem->get_static_initial_literals());
    auto static_assignment_set = AssignmentSet<Static>(m_problem, m_problem->get_domain()->get_predicates<Static>(), static_initial_atoms);

    for (const auto& action : m_problem->get_domain()->get_actions())
    {
        m_action_precondition_grounders.emplace(action,
                                                ConditionGrounder(m_problem,
                                                                  m_pddl_repositories,
                                                                  action->get_parameters(),
                                                                  action->get_conditions<Static>(),
                                                                  action->get_conditions<Fluent>(),
                                                                  action->get_conditions<Derived>(),
                                                                  static_assignment_set));
        auto conditional_effects = std::vector<consistency_graph::StaticConsistencyGraph>();
        conditional_effects.reserve(action->get_conditional_effects().size());

        for (const auto& conditional_effect : action->get_conditional_effects())
        {
            conditional_effects.push_back(consistency_graph::StaticConsistencyGraph(problem,
                                                                                    action->get_arity(),
                                                                                    action->get_arity() + conditional_effect->get_arity(),
                                                                                    conditional_effect->get_conditions<Static>(),
                                                                                    static_assignment_set));
        }

        m_action_conditional_effects.emplace(action, std::move(conditional_effects));
    }
}

Problem ActionGrounder::get_problem() const { return m_problem; }

const std::shared_ptr<PDDLRepositories>& ActionGrounder::get_pddl_repositories() const { return m_pddl_repositories; }

std::unordered_map<Action, ConditionGrounder>& ActionGrounder::get_action_precondition_grounders() { return m_action_precondition_grounders; }

GroundAction ActionGrounder::ground_action(Action action, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& groundings = m_action_groundings[action];
    auto it = groundings.find(binding);
    if (it != groundings.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

    // m_event_handler->on_ground_action(action, binding);

    const auto fill_effects = [this](const LiteralList<Fluent>& lifted_effect_literals,  //
                                     const ObjectList& binding,
                                     GroundEffectFluentLiteralList& out_grounded_effect_literals)
    {
        out_grounded_effect_literals.clear();

        for (const auto& lifted_literal : lifted_effect_literals)
        {
            const auto grounded_literal = m_pddl_repositories->ground_literal(lifted_literal, binding);
            out_grounded_effect_literals.emplace_back(grounded_literal->is_negated(), grounded_literal->get_atom()->get_index());
        }
    };

    /* Header */

    m_action_builder.get_index() = m_actions.size();
    m_action_builder.get_action_index() = action->get_index();
    auto& objects = m_action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }

    /* Precondition */
    auto& strips_precondition = m_action_builder.get_strips_precondition();
    auto& positive_fluent_precondition = strips_precondition.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = strips_precondition.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = strips_precondition.get_positive_precondition<Static>();
    auto& negative_static_precondition = strips_precondition.get_negative_precondition<Static>();
    auto& positive_derived_precondition = strips_precondition.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = strips_precondition.get_negative_precondition<Derived>();
    positive_fluent_precondition.unset_all();
    negative_fluent_precondition.unset_all();
    positive_static_precondition.unset_all();
    negative_static_precondition.unset_all();
    positive_derived_precondition.unset_all();
    negative_derived_precondition.unset_all();
    m_pddl_repositories->ground_and_fill_bitset(action->get_conditions<Fluent>(), positive_fluent_precondition, negative_fluent_precondition, binding);
    m_pddl_repositories->ground_and_fill_bitset(action->get_conditions<Static>(), positive_static_precondition, negative_static_precondition, binding);
    m_pddl_repositories->ground_and_fill_bitset(action->get_conditions<Derived>(), positive_derived_precondition, negative_derived_precondition, binding);

    /* Simple effects */
    auto& strips_effect = m_action_builder.get_strips_effect();
    auto& positive_effect = strips_effect.get_positive_effects();
    auto& negative_effect = strips_effect.get_negative_effects();
    positive_effect.unset_all();
    negative_effect.unset_all();
    const auto& lifted_strips_effect = action->get_strips_effect();
    const auto& lifted_effect_literals = lifted_strips_effect->get_effects();
    m_pddl_repositories->ground_and_fill_bitset(lifted_effect_literals, positive_effect, negative_effect, binding);
    strips_effect.get_cost() =
        GroundAndEvaluateFunctionExpressionVisitor(m_ground_function_to_cost, binding, *m_pddl_repositories)(*lifted_strips_effect->get_function_expression());

    /* Conditional effects */
    // Fetch data
    auto& conditional_effects = m_action_builder.get_conditional_effects();
    // TODO: Unfortunately, this unnecessary causes deallocations. We need to write a custom cista vector that avoids this.
    conditional_effects.clear();

    // Resize builders.
    /* Universal effects */

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_ext = binding;

    const auto num_lifted_conditional_effects = action->get_conditional_effects().size();
    if (num_lifted_conditional_effects > 0)
    {
        const auto& conditional_effect_consistency_graphs = m_action_conditional_effects.at(action);
        const auto binding_ext_size = binding_ext.size();
        for (size_t i = 0; i < num_lifted_conditional_effects; ++i)
        {
            // Fetch data
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            // Resize builders.
            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& consistency_graph = conditional_effect_consistency_graphs.at(i);
                const auto& objects_by_parameter_index = consistency_graph.get_objects_by_parameter_index();

                const auto num_conditional_effects = CartesianProduct(objects_by_parameter_index).num_combinations();
                const auto old_size = conditional_effects.size();
                conditional_effects.resize(old_size + num_conditional_effects);

                // Create binding and ground conditions and effect
                binding_ext.resize(binding_ext_size + lifted_cond_effect->get_arity());

                // The position to place the conditional precondition + effect
                auto j = old_size;
                assert(!objects_by_parameter_index.empty());
                for (const auto& combination : CartesianProduct(objects_by_parameter_index))
                {
                    // Create binding
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        const auto object_index = *combination[pos];
                        binding_ext[binding_ext_size + pos] = m_pddl_repositories->get_object(object_index);
                    }

                    auto& cond_effect_j = conditional_effects[j];
                    auto& cond_positive_fluent_precondition_j = cond_effect_j.get_positive_precondition<Fluent>();
                    auto& cond_negative_fluent_precondition_j = cond_effect_j.get_negative_precondition<Fluent>();
                    auto& cond_positive_static_precondition_j = cond_effect_j.get_positive_precondition<Static>();
                    auto& cond_negative_static_precondition_j = cond_effect_j.get_negative_precondition<Static>();
                    auto& cond_positive_derived_precondition_j = cond_effect_j.get_positive_precondition<Derived>();
                    auto& cond_negative_derived_precondition_j = cond_effect_j.get_negative_precondition<Derived>();
                    auto& cond_simple_effect_j = cond_effect_j.get_fluent_effect_literals();
                    cond_positive_fluent_precondition_j.clear();
                    cond_negative_fluent_precondition_j.clear();
                    cond_positive_static_precondition_j.clear();
                    cond_negative_static_precondition_j.clear();
                    cond_positive_derived_precondition_j.clear();
                    cond_negative_derived_precondition_j.clear();
                    m_pddl_repositories->ground_and_fill_vector(lifted_cond_effect->get_conditions<Fluent>(),
                                                                cond_positive_fluent_precondition_j,
                                                                cond_negative_fluent_precondition_j,
                                                                binding_ext);
                    m_pddl_repositories->ground_and_fill_vector(lifted_cond_effect->get_conditions<Static>(),
                                                                cond_positive_static_precondition_j,
                                                                cond_negative_static_precondition_j,
                                                                binding_ext);
                    m_pddl_repositories->ground_and_fill_vector(lifted_cond_effect->get_conditions<Derived>(),
                                                                cond_positive_derived_precondition_j,
                                                                cond_negative_derived_precondition_j,
                                                                binding_ext);

                    fill_effects(lifted_cond_effect->get_effects(), binding_ext, cond_simple_effect_j);

                    cond_effect_j.get_cost() = GroundAndEvaluateFunctionExpressionVisitor(m_ground_function_to_cost, binding, *m_pddl_repositories)(
                        *lifted_cond_effect->get_function_expression());

                    ++j;
                }
            }
            else
            {
                conditional_effects.resize(conditional_effects.size() + 1);
                auto& cond_effect = conditional_effects.back();
                auto& cond_positive_fluent_precondition = cond_effect.get_positive_precondition<Fluent>();
                auto& cond_negative_fluent_precondition = cond_effect.get_negative_precondition<Fluent>();
                auto& cond_positive_static_precondition = cond_effect.get_positive_precondition<Static>();
                auto& cond_negative_static_precondition = cond_effect.get_negative_precondition<Static>();
                auto& cond_positive_derived_precondition = cond_effect.get_positive_precondition<Derived>();
                auto& cond_negative_derived_precondition = cond_effect.get_negative_precondition<Derived>();
                auto& cond_simple_effect = cond_effect.get_fluent_effect_literals();
                cond_positive_fluent_precondition.clear();
                cond_negative_fluent_precondition.clear();
                cond_positive_static_precondition.clear();
                cond_negative_static_precondition.clear();
                cond_positive_derived_precondition.clear();
                cond_negative_derived_precondition.clear();
                m_pddl_repositories->ground_and_fill_vector(lifted_cond_effect->get_conditions<Fluent>(),
                                                            cond_positive_fluent_precondition,
                                                            cond_negative_fluent_precondition,
                                                            binding);
                m_pddl_repositories->ground_and_fill_vector(lifted_cond_effect->get_conditions<Static>(),
                                                            cond_positive_static_precondition,
                                                            cond_negative_static_precondition,
                                                            binding);
                m_pddl_repositories->ground_and_fill_vector(lifted_cond_effect->get_conditions<Derived>(),
                                                            cond_positive_derived_precondition,
                                                            cond_negative_derived_precondition,
                                                            binding);

                fill_effects(lifted_cond_effect->get_effects(), binding, cond_simple_effect);

                cond_effect.get_cost() = GroundAndEvaluateFunctionExpressionVisitor(m_ground_function_to_cost, binding, *m_pddl_repositories)(
                    *lifted_cond_effect->get_function_expression());
            }
        }
    }

    const auto [iter, inserted] = m_actions.insert(m_action_builder);
    const auto grounded_action = *iter;
    if (inserted)
    {
        m_actions_by_index.push_back(grounded_action);
    }

    /* 3. Insert to groundings table */

    groundings.emplace(std::move(binding), GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

const GroundActionList& ActionGrounder::get_ground_actions() const { return m_actions_by_index; }

GroundAction ActionGrounder::get_ground_action(Index action_index) const { return m_actions_by_index.at(action_index); }

size_t ActionGrounder::get_num_ground_actions() const { return m_actions_by_index.size(); }
}
