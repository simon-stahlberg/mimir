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
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/search/action.hpp"
#include "mimir/search/grounders/grounder.hpp"

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
        return m_problem->get_ground_function_value(m_function_grounder.ground_function(expr.get_function(), m_binding));
    }
};

/// @brief Simplest construction
ActionGrounder::ActionGrounder(std::shared_ptr<LiteralGrounder> literal_grounder, std::shared_ptr<FunctionGrounder> function_grounder) :
    m_literal_grounder(std::move(literal_grounder)),
    m_function_grounder(std::move(function_grounder)),
    m_actions(),
    m_actions_by_index(),
    m_action_builder(),
    m_action_groundings(),
    m_action_conditional_effects()
{
    /* 1. Initialize ground function costs. */

    const auto problem = m_literal_grounder->get_problem();

    /* 2. Initialize the condition grounders for each action schema. */

    auto static_initial_atoms = to_ground_atoms(problem->get_static_initial_literals());
    auto static_assignment_set = AssignmentSet<Static>(problem, problem->get_domain()->get_predicates<Static>(), static_initial_atoms);

    for (const auto& action : problem->get_domain()->get_actions())
    {
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

GroundAction ActionGrounder::ground_action(Action action, ObjectList binding)
{
    const auto problem = m_literal_grounder->get_problem();

    /* 1. Check if grounding is cached */

    auto& groundings = m_action_groundings[action];
    auto it = groundings.find(binding);
    if (it != groundings.end())
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

    m_action_builder.get_index() = m_actions.size();
    m_action_builder.get_action_index() = action->get_index();
    auto& objects = m_action_builder.get_objects();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }

    /* Strips precondition */
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
    m_literal_grounder->ground_and_fill_bitset(action->get_conditions<Fluent>(), positive_fluent_precondition, negative_fluent_precondition, binding);
    m_literal_grounder->ground_and_fill_bitset(action->get_conditions<Static>(), positive_static_precondition, negative_static_precondition, binding);
    m_literal_grounder->ground_and_fill_bitset(action->get_conditions<Derived>(), positive_derived_precondition, negative_derived_precondition, binding);

    /* Strips effects */
    auto& strips_effect = m_action_builder.get_strips_effect();
    auto& positive_effect = strips_effect.get_positive_effects();
    auto& negative_effect = strips_effect.get_negative_effects();
    positive_effect.unset_all();
    negative_effect.unset_all();
    const auto& lifted_strips_effect = action->get_strips_effect();
    const auto& lifted_effect_literals = lifted_strips_effect->get_effects();
    m_literal_grounder->ground_and_fill_bitset(lifted_effect_literals, positive_effect, negative_effect, binding);
    strips_effect.get_cost() =
        GroundAndEvaluateFunctionExpressionVisitor(problem, binding, *m_function_grounder)(*lifted_strips_effect->get_function_expression());

    /* Conditional effects */
    // Fetch data
    auto& conditional_effects = m_action_builder.get_conditional_effects();
    conditional_effects.clear();

    // We have copy the binding to extend it with objects for quantified effect variables
    // and at the same time we need to use the original binding as cache key.
    auto binding_ext = binding;

    const auto num_lifted_conditional_effects = action->get_conditional_effects().size();
    if (num_lifted_conditional_effects > 0)
    {
        const auto& conditional_effect_consistency_graphs = m_action_conditional_effects.at(action);
        for (size_t i = 0; i < num_lifted_conditional_effects; ++i)
        {
            // Fetch data
            const auto& lifted_cond_effect = action->get_conditional_effects().at(i);

            if (lifted_cond_effect->get_arity() > 0)
            {
                const auto& consistency_graph = conditional_effect_consistency_graphs.at(i);
                const auto& objects_by_parameter_index = consistency_graph.get_objects_by_parameter_index();

                // Create binding that also contains bindings of variables in quantified effect.
                binding_ext.resize(binding.size() + lifted_cond_effect->get_arity());

                for (const auto& binding_cond : create_cartesian_product_generator(objects_by_parameter_index))
                {
                    // Create binding
                    for (size_t pos = 0; pos < lifted_cond_effect->get_arity(); ++pos)
                    {
                        binding_ext[binding.size() + pos] = get_pddl_repositories()->get_object(binding_cond[pos]);
                    }

                    auto cond_effect_j = GroundEffectConditional();
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
                                                               binding_ext);
                    m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Static>(),
                                                               cond_positive_static_precondition_j,
                                                               cond_negative_static_precondition_j,
                                                               binding_ext);
                    m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_conditions<Derived>(),
                                                               cond_positive_derived_precondition_j,
                                                               cond_negative_derived_precondition_j,
                                                               binding_ext);

                    fill_effects(lifted_cond_effect->get_effects(), binding_ext, cond_simple_effect_j);

                    cond_effect_j.get_cost() =
                        GroundAndEvaluateFunctionExpressionVisitor(problem, binding, *m_function_grounder)(*lifted_cond_effect->get_function_expression());

                    conditional_effects.push_back(std::move(cond_effect_j));
                }
            }
            else
            {
                auto cond_effect = GroundEffectConditional();
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

                fill_effects(lifted_cond_effect->get_effects(), binding, cond_simple_effect);

                cond_effect.get_cost() =
                    GroundAndEvaluateFunctionExpressionVisitor(problem, binding, *m_function_grounder)(*lifted_cond_effect->get_function_expression());

                conditional_effects.push_back(std::move(cond_effect));
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

Problem ActionGrounder::get_problem() const { return m_literal_grounder->get_problem(); }

const std::shared_ptr<PDDLRepositories>& ActionGrounder::get_pddl_repositories() const { return m_literal_grounder->get_pddl_repositories(); }

const std::shared_ptr<LiteralGrounder>& ActionGrounder::get_literal_grounder() const { return m_literal_grounder; }

const std::shared_ptr<FunctionGrounder>& ActionGrounder::get_function_grounder() const { return m_function_grounder; }

const GroundActionList& ActionGrounder::get_ground_actions() const { return m_actions_by_index; }

GroundAction ActionGrounder::get_ground_action(Index action_index) const { return m_actions_by_index.at(action_index); }

size_t ActionGrounder::get_num_ground_actions() const { return m_actions_by_index.size(); }
}
