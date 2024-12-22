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

#include "mimir/search/grounders/action_grounder.hpp"

#include "mimir/common/itertools.hpp"
#include "mimir/common/memory.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/grounders/function_grounder.hpp"
#include "mimir/search/grounders/grounder.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"

#include <bit>

namespace mimir
{

class GroundAndEvaluateFunctionExpressionVisitor
{
private:
    Problem m_problem;
    const ObjectList& m_binding;
    FunctionGrounder& m_function_grounder;

public:
    GroundAndEvaluateFunctionExpressionVisitor(Problem problem, const ObjectList& binding, FunctionGrounder& ref_function_grounder) :
        m_problem(problem),
        m_binding(binding),
        m_function_grounder(ref_function_grounder)
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
        const auto op = expr.get_multi_operator();
        return std::accumulate(std::next(expr.get_function_expressions().begin()),  // Start from the second expression
                               expr.get_function_expressions().end(),
                               (*this)(*expr.get_function_expressions().front()),  // Initial bounds
                               [this, op](const auto& value, const auto& child_expr) { return evaluate_multi(op, value, (*this)(*child_expr)); });
    }

    double operator()(const FunctionExpressionMinusImpl& expr) { return -(*this)(*expr.get_function_expression()); }

    double operator()(const FunctionExpressionFunctionImpl& expr)
    {
        return m_problem->get_ground_function_value(m_function_grounder.ground_function(expr.get_function(), m_binding));
    }
};

/// @brief Simplest construction
ActionGrounder::ActionGrounder(std::shared_ptr<LiteralGrounder> literal_grounder, std::shared_ptr<FunctionGrounder> function_grounder) :
    m_literal_grounder(std::move(literal_grounder)),
    m_function_grounder(std::move(function_grounder)),
    m_actions(),
    m_actions_by_index(),
    m_per_action_datas()
{
    /* 1. Initialize ground function costs. */

    const auto problem = m_literal_grounder->get_problem();

    /* 2. Initialize the per_action_data for each action schema. */

    for (const auto& action : problem->get_domain()->get_actions())
    {
        auto cond_effect_static_consistency_graphs = std::vector<consistency_graph::StaticConsistencyGraph>();
        cond_effect_static_consistency_graphs.reserve(action->get_conditional_effects().size());

        auto action_builder = GroundActionImpl();
        size_t num_cond_effects = 0;  ///< Preallocate enough conditional effects to avoid frequent allocations during grounding.

        for (const auto& conditional_effect : action->get_conditional_effects())
        {
            auto static_consistency_graph = consistency_graph::StaticConsistencyGraph(problem,
                                                                                      action->get_arity(),
                                                                                      action->get_arity() + conditional_effect->get_arity(),
                                                                                      conditional_effect->get_conditions<Static>());

            num_cond_effects +=
                (conditional_effect->get_arity() > 0) ? get_size_cartesian_product(static_consistency_graph.get_objects_by_parameter_index()) : 1;

            cond_effect_static_consistency_graphs.push_back(std::move(static_consistency_graph));
        }

        action_builder.get_conditional_effects().resize(num_cond_effects);

        m_per_action_datas.emplace(
            action,
            std::make_tuple(std::move(action_builder), GroundingTable<GroundAction>(), std::move(cond_effect_static_consistency_graphs)));
    }
}

GroundAction ActionGrounder::ground_action(Action action, ObjectList binding)
{
    const auto problem = m_literal_grounder->get_problem();

    /* 1. Check if grounding is cached */

    auto& [action_builder, grounding_table, cond_effect_static_consistency_graphs] = m_per_action_datas[action];

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

    const auto fill_effects = [this](const LiteralList<Fluent>& lifted_effect_literals,  //
                                     const ObjectList& binding,
                                     GroundEffectFluentLiteralList& out_grounded_effect_literals)
    {
        out_grounded_effect_literals.clear();

        for (const auto& lifted_literal : lifted_effect_literals)
        {
            const auto grounded_literal = m_literal_grounder->ground_literal(lifted_literal, binding);
            out_grounded_effect_literals.emplace_back(grounded_literal->is_negated(), grounded_literal->get_atom()->get_index());
        }
    };

    /* Header */

    action_builder.get_index() = m_actions.size();
    action_builder.get_action_index() = action->get_index();
    auto& objects = action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Strips precondition */
    auto& strips_precondition = action_builder.get_strips_precondition();
    auto& positive_fluent_precondition = strips_precondition.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = strips_precondition.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = strips_precondition.get_positive_precondition<Static>();
    auto& negative_static_precondition = strips_precondition.get_negative_precondition<Static>();
    auto& positive_derived_precondition = strips_precondition.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = strips_precondition.get_negative_precondition<Derived>();
    positive_fluent_precondition.clear();
    negative_fluent_precondition.clear();
    positive_static_precondition.clear();
    negative_static_precondition.clear();
    positive_derived_precondition.clear();
    negative_derived_precondition.clear();
    m_literal_grounder->ground_and_fill_vector(action->get_precondition()->get_literals<Fluent>(),
                                               positive_fluent_precondition,
                                               negative_fluent_precondition,
                                               binding);
    m_literal_grounder->ground_and_fill_vector(action->get_precondition()->get_literals<Static>(),
                                               positive_static_precondition,
                                               negative_static_precondition,
                                               binding);
    m_literal_grounder->ground_and_fill_vector(action->get_precondition()->get_literals<Derived>(),
                                               positive_derived_precondition,
                                               negative_derived_precondition,
                                               binding);
    positive_fluent_precondition.compress();
    negative_fluent_precondition.compress();
    positive_static_precondition.compress();
    negative_static_precondition.compress();
    positive_derived_precondition.compress();
    negative_derived_precondition.compress();

    /* Strips effects */
    auto& strips_effect = action_builder.get_strips_effect();
    auto& positive_effect = strips_effect.get_positive_effects();
    auto& negative_effect = strips_effect.get_negative_effects();
    positive_effect.clear();
    negative_effect.clear();
    const auto& lifted_strips_effect = action->get_strips_effect();
    const auto& lifted_effect_literals = lifted_strips_effect->get_effects();
    m_literal_grounder->ground_and_fill_vector(lifted_effect_literals, positive_effect, negative_effect, binding);
    positive_effect.compress();
    negative_effect.compress();

    strips_effect.get_cost() =
        GroundAndEvaluateFunctionExpressionVisitor(problem, binding, *m_function_grounder)(*lifted_strips_effect->get_function_expression());

    /* Conditional effects */
    // Fetch data
    auto& cond_effects = action_builder.get_conditional_effects();

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_cond_effect = binding;

    const auto num_lifted_cond_effects = action->get_conditional_effects().size();
    if (num_lifted_cond_effects > 0)
    {
        size_t j = 0;  ///< position in cond_effects

        for (size_t i = 0; i < num_lifted_cond_effects; ++i)
        {
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& cond_effect_static_consistency_graph = cond_effect_static_consistency_graphs.at(i);
                const auto& objects_by_parameter_index = cond_effect_static_consistency_graph.get_objects_by_parameter_index();

                // Resize binding to have additional space for all variables in quantified effect.
                binding_cond_effect.resize(binding.size() + lifted_cond_effect->get_arity());

                for (const auto& binding_cond : create_cartesian_product_generator(objects_by_parameter_index))
                {
                    // Create resulting conditional effect binding.
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        binding_cond_effect[binding.size() + pos] = get_pddl_repositories()->get_object(binding_cond[pos]);
                    }

                    auto& cond_effect_j = cond_effects.at(j++);
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
                    m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Fluent>(),
                                                               cond_positive_fluent_precondition_j,
                                                               cond_negative_fluent_precondition_j,
                                                               binding_cond_effect);
                    m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Static>(),
                                                               cond_positive_static_precondition_j,
                                                               cond_negative_static_precondition_j,
                                                               binding_cond_effect);
                    m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Derived>(),
                                                               cond_positive_derived_precondition_j,
                                                               cond_negative_derived_precondition_j,
                                                               binding_cond_effect);
                    cond_positive_fluent_precondition_j.compress();
                    cond_negative_fluent_precondition_j.compress();
                    cond_positive_static_precondition_j.compress();
                    cond_negative_static_precondition_j.compress();
                    cond_positive_derived_precondition_j.compress();
                    cond_negative_derived_precondition_j.compress();

                    fill_effects(lifted_cond_effect->get_effects(), binding_cond_effect, cond_simple_effect_j);

                    cond_effect_j.get_cost() =
                        GroundAndEvaluateFunctionExpressionVisitor(problem, binding, *m_function_grounder)(*lifted_cond_effect->get_function_expression());
                }
            }
            else
            {
                auto& cond_effect = cond_effects.at(j++);
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
                m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Fluent>(),
                                                           cond_positive_fluent_precondition,
                                                           cond_negative_fluent_precondition,
                                                           binding);
                m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Static>(),
                                                           cond_positive_static_precondition,
                                                           cond_negative_static_precondition,
                                                           binding);
                m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Derived>(),
                                                           cond_positive_derived_precondition,
                                                           cond_negative_derived_precondition,
                                                           binding);
                cond_positive_fluent_precondition.compress();
                cond_negative_fluent_precondition.compress();
                cond_positive_static_precondition.compress();
                cond_negative_static_precondition.compress();
                cond_positive_derived_precondition.compress();
                cond_negative_derived_precondition.compress();

                fill_effects(lifted_cond_effect->get_effects(), binding, cond_simple_effect);

                cond_effect.get_cost() =
                    GroundAndEvaluateFunctionExpressionVisitor(problem, binding, *m_function_grounder)(*lifted_cond_effect->get_function_expression());
            }
        }
    }

    const auto [iter, inserted] = m_actions.insert(action_builder);
    const auto grounded_action = iter->get();
    if (inserted)
    {
        m_actions_by_index.push_back(grounded_action);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAction(grounded_action));

    /* 4. Return the resulting ground action */

    return grounded_action;
}

Problem ActionGrounder::get_problem() const { return m_literal_grounder->get_problem(); }

const std::shared_ptr<PDDLRepositories>& ActionGrounder::get_pddl_repositories() const { return m_literal_grounder->get_pddl_repositories(); }

const std::shared_ptr<LiteralGrounder>& ActionGrounder::get_literal_grounder() const { return m_literal_grounder; }

const std::shared_ptr<FunctionGrounder>& ActionGrounder::get_function_grounder() const { return m_function_grounder; }

const GroundActionList& ActionGrounder::get_ground_actions() const { return m_actions_by_index; }

GroundAction ActionGrounder::get_ground_action(Index action_index) const { return m_actions_by_index.at(action_index); }

size_t ActionGrounder::get_num_ground_actions() const { return m_actions_by_index.size(); }

size_t ActionGrounder::get_estimated_memory_usage_in_bytes_for_actions() const
{
    const auto usage1 = m_actions.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_actions_by_index.capacity() * sizeof(GroundAction);
    auto usage3 = size_t(0);
    // TODO: add memory usage of m_per_action_datas
    for (const auto& [action, action_data] : m_per_action_datas)
    {
        const auto& [action_builder, grounding_table, static_consistency_graph] = action_data;
        // TODO: add memory usage of action_builder

        usage3 += get_memory_usage_in_bytes(grounding_table);

        // TODO: add memory usage of static_consistency_graph
    }

    return usage1 + usage2 + usage3;
}
}
