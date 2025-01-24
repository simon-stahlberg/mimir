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

#include "mimir/formalism/grounders/action_grounder.hpp"

#include "mimir/common/itertools.hpp"
#include "mimir/common/memory.hpp"
#include "mimir/formalism/grounders/function_grounder.hpp"
#include "mimir/formalism/grounders/grounder.hpp"
#include "mimir/formalism/grounders/literal_grounder.hpp"
#include "mimir/formalism/grounders/numeric_constraint_grounder.hpp"
#include "mimir/formalism/grounders/numeric_effect_grounder.hpp"
#include "mimir/formalism/repositories.hpp"

#include <bit>

namespace mimir
{

/// @brief Simplest construction
ActionGrounder::ActionGrounder(std::shared_ptr<LiteralGrounder> literal_grounder,
                               std::shared_ptr<NumericConstraintGrounder> numeric_constraint_grounder,
                               std::shared_ptr<NumericEffectGrounder> numeric_effect_grounder) :
    m_literal_grounder(std::move(literal_grounder)),
    m_numeric_constraint_grounder(std::move(numeric_constraint_grounder)),
    m_numeric_effect_grounder(std::move(numeric_effect_grounder)),
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

void ActionGrounder::ground_and_fill_vector(const NumericConstraintList& numeric_constraints,
                                            FlatExternalPtrList<const GroundNumericConstraintImpl>& ref_numeric_constraints,
                                            const ObjectList& binding)
{
    for (const auto& condition : numeric_constraints)
    {
        ref_numeric_constraints.push_back(m_numeric_constraint_grounder->ground(condition, binding));
    }
}

template<DynamicFunctionTag F>
void ActionGrounder::ground_and_fill_vector(const EffectNumericList<F>& numeric_effects,
                                            GroundEffectNumericList<F>& ref_numeric_effects,
                                            const ObjectList& binding)
{
    for (const auto& effect : numeric_effects)
    {
        ref_numeric_effects.push_back(m_numeric_effect_grounder->ground(effect, binding));
    }
}

template void ActionGrounder::ground_and_fill_vector(const EffectNumericList<Fluent>& numeric_effects,
                                                     GroundEffectNumericList<Fluent>& ref_numeric_effects,
                                                     const ObjectList& binding);
template void ActionGrounder::ground_and_fill_vector(const EffectNumericList<Auxiliary>& numeric_effects,
                                                     GroundEffectNumericList<Auxiliary>& ref_numeric_effects,
                                                     const ObjectList& binding);

GroundAction ActionGrounder::ground(Action action, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& [action_builder, grounding_table, cond_effect_static_consistency_graphs] = m_per_action_datas[action];

    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the action */

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
    auto& numeric_constraints = strips_precondition.get_numeric_constraints();
    positive_fluent_precondition.clear();
    negative_fluent_precondition.clear();
    positive_static_precondition.clear();
    negative_static_precondition.clear();
    positive_derived_precondition.clear();
    negative_derived_precondition.clear();
    numeric_constraints.clear();
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
    ground_and_fill_vector(action->get_precondition()->get_numeric_constraints(), numeric_constraints, binding);

    /* Strips propositional effects */
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

    /* Strips numerical effects */
    auto& fluent_numerical_effects = strips_effect.get_numeric_effects<Fluent>();
    auto& auxiliary_numerical_effects = strips_effect.get_numeric_effects<Auxiliary>();
    fluent_numerical_effects.clear();
    auxiliary_numerical_effects.clear();
    ground_and_fill_vector(lifted_strips_effect->get_numeric_effects<Fluent>(), fluent_numerical_effects, binding);
    ground_and_fill_vector(lifted_strips_effect->get_numeric_effects<Auxiliary>(), auxiliary_numerical_effects, binding);

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
                    auto& cond_strips_precondition_j = cond_effect_j.get_strips_precondition();
                    auto& cond_positive_fluent_precondition_j = cond_strips_precondition_j.get_positive_precondition<Fluent>();
                    auto& cond_negative_fluent_precondition_j = cond_strips_precondition_j.get_negative_precondition<Fluent>();
                    auto& cond_positive_static_precondition_j = cond_strips_precondition_j.get_positive_precondition<Static>();
                    auto& cond_negative_static_precondition_j = cond_strips_precondition_j.get_negative_precondition<Static>();
                    auto& cond_positive_derived_precondition_j = cond_strips_precondition_j.get_positive_precondition<Derived>();
                    auto& cond_negative_derived_precondition_j = cond_strips_precondition_j.get_negative_precondition<Derived>();
                    auto& cond_numeric_constraints_j = cond_strips_precondition_j.get_numeric_constraints();
                    auto& cond_strips_effect_j = cond_effect_j.get_strips_effect();
                    auto& cond_positive_effect_j = cond_strips_effect_j.get_positive_effects();
                    auto& cond_negative_effect_j = cond_strips_effect_j.get_negative_effects();
                    cond_positive_fluent_precondition_j.clear();
                    cond_negative_fluent_precondition_j.clear();
                    cond_positive_static_precondition_j.clear();
                    cond_negative_static_precondition_j.clear();
                    cond_positive_derived_precondition_j.clear();
                    cond_negative_derived_precondition_j.clear();
                    cond_numeric_constraints_j.clear();
                    cond_positive_effect_j.clear();
                    cond_negative_effect_j.clear();

                    /* Propositional precondition */
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
                    ground_and_fill_vector(lifted_cond_effect->get_numeric_constraints(), cond_numeric_constraints_j, binding_cond_effect);

                    /* Propositional effect */
                    m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_effects(),
                                                               cond_positive_effect_j,
                                                               cond_negative_effect_j,
                                                               binding_cond_effect);

                    /* Numeric effect */
                    auto& cond_fluent_numerical_effects_j = strips_effect.get_numeric_effects<Fluent>();
                    auto& cond_auxiliary_numerical_effects_j = strips_effect.get_numeric_effects<Auxiliary>();
                    cond_fluent_numerical_effects_j.clear();
                    cond_auxiliary_numerical_effects_j.clear();

                    ground_and_fill_vector(lifted_cond_effect->get_numeric_effects<Fluent>(), cond_fluent_numerical_effects_j, binding_cond_effect);
                    ground_and_fill_vector(lifted_cond_effect->get_numeric_effects<Auxiliary>(), cond_auxiliary_numerical_effects_j, binding_cond_effect);
                }
            }
            else
            {
                auto& cond_effect = cond_effects.at(j++);
                auto& cond_strips_precondition = cond_effect.get_strips_precondition();
                auto& cond_positive_fluent_precondition = cond_strips_precondition.get_positive_precondition<Fluent>();
                auto& cond_negative_fluent_precondition = cond_strips_precondition.get_negative_precondition<Fluent>();
                auto& cond_positive_static_precondition = cond_strips_precondition.get_positive_precondition<Static>();
                auto& cond_negative_static_precondition = cond_strips_precondition.get_negative_precondition<Static>();
                auto& cond_positive_derived_precondition = cond_strips_precondition.get_positive_precondition<Derived>();
                auto& cond_negative_derived_precondition = cond_strips_precondition.get_negative_precondition<Derived>();
                auto& cond_numeric_constraints = cond_strips_precondition.get_numeric_constraints();
                auto& cond_strips_effect = cond_effect.get_strips_effect();
                auto& cond_positive_effect = cond_strips_effect.get_positive_effects();
                auto& cond_negative_effect = cond_strips_effect.get_negative_effects();
                cond_positive_fluent_precondition.clear();
                cond_negative_fluent_precondition.clear();
                cond_positive_static_precondition.clear();
                cond_negative_static_precondition.clear();
                cond_positive_derived_precondition.clear();
                cond_negative_derived_precondition.clear();
                cond_numeric_constraints.clear();
                cond_positive_effect.clear();
                cond_negative_effect.clear();

                /* Propositional precondition */
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
                ground_and_fill_vector(lifted_cond_effect->get_numeric_constraints(), cond_numeric_constraints, binding);

                /* Propositional effect */
                m_literal_grounder->ground_and_fill_vector(lifted_cond_effect->get_effects(), cond_positive_effect, cond_negative_effect, binding);

                /* Numeric effect*/
                auto& cond_fluent_numerical_effects_j = strips_effect.get_numeric_effects<Fluent>();
                auto& cond_auxiliary_numerical_effects_j = strips_effect.get_numeric_effects<Auxiliary>();
                cond_fluent_numerical_effects_j.clear();
                cond_auxiliary_numerical_effects_j.clear();

                ground_and_fill_vector(lifted_cond_effect->get_numeric_effects<Fluent>(), cond_fluent_numerical_effects_j, binding);
                ground_and_fill_vector(lifted_cond_effect->get_numeric_effects<Auxiliary>(), cond_auxiliary_numerical_effects_j, binding);
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

const std::shared_ptr<NumericConstraintGrounder>& ActionGrounder::get_numeric_constraint_grounder() const { return m_numeric_constraint_grounder; }

const std::shared_ptr<NumericEffectGrounder>& ActionGrounder::get_numeric_effect_grounder() const { return m_numeric_effect_grounder; }

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
