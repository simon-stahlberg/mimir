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

#include "mimir/formalism/transformers/positive_normal_form.hpp"

namespace mimir
{
template<PredicateCategory P>
static void collect_negative_conditions(const LiteralList<P>& conditions, LiteralSet<P>& ref_negative_conditions)
{
    for (const auto& literal : conditions)
    {
        if (literal->is_negated())
        {
            ref_negative_conditions.insert(literal);
        }
    }
}

void PositiveNormalFormTransformer::prepare_impl(const LiteralImpl<Static>& literal) { m_static_literals.insert(&literal); }

void PositiveNormalFormTransformer::prepare_impl(const LiteralImpl<Fluent>& literal) { m_fluent_literals.insert(&literal); }

void PositiveNormalFormTransformer::prepare_impl(const EffectConditionalImpl& effect)
{
    collect_negative_conditions(effect.get_conditions<Static>(), m_negative_static_conditions);
    collect_negative_conditions(effect.get_conditions<Fluent>(), m_negative_fluent_conditions);
}

void PositiveNormalFormTransformer::prepare_impl(const EffectUniversalImpl& effect)
{
    collect_negative_conditions(effect.get_conditions<Static>(), m_negative_static_conditions);
    collect_negative_conditions(effect.get_conditions<Fluent>(), m_negative_fluent_conditions);
}

void PositiveNormalFormTransformer::prepare_impl(const ActionImpl& action)
{
    collect_negative_conditions(action.get_conditions<Static>(), m_negative_static_conditions);
    collect_negative_conditions(action.get_conditions<Fluent>(), m_negative_fluent_conditions);
}

Action PositiveNormalFormTransformer::transform_impl(const ActionImpl& action)
{
    const auto transform_fluent_effect_literal = [&, this](const Literal<Fluent>& literal, LiteralList<Fluent>& out_literals)
    {
        out_literals.clear();

        const auto transformed_literal = this->transform(*literal);
        out_literals.push_back(transformed_literal);

        if (this->m_negative_fluent_duals.count(literal))
        {
            out_literals.push_back(this->m_negative_fluent_duals.at(literal));
        }
        else if (this->m_positive_fluent_duals.count(literal))
        {
            out_literals.push_back(this->m_positive_fluent_duals.at(literal));
        }
    };

    auto transformed_static_conditions = LiteralList<Static> {};
    auto transformed_fluent_conditions = LiteralList<Fluent> {};
    transform_conditions(action.get_conditions<Static>(), m_negative_static_duals, transformed_static_conditions);
    transform_conditions(action.get_conditions<Fluent>(), m_negative_fluent_duals, transformed_fluent_conditions);

    auto transformed_simple_effects = EffectSimpleList {};
    auto transformed_effect_literals = LiteralList<Fluent> {};
    for (const auto& effect : action.get_simple_effects())
    {
        transform_fluent_effect_literal(effect->get_effect(), transformed_effect_literals);
        for (const auto& transformed_literal : transformed_effect_literals)
        {
            transformed_simple_effects.push_back(this->m_pddl_factories.get_or_create_simple_effect(transformed_literal));
        }
    }

    auto transformed_conditional_effects = EffectConditionalList {};
    auto transformed_action_static_conditions = LiteralList<Static> {};
    auto transformed_action_fluent_conditions = LiteralList<Fluent> {};
    for (const auto& effect : action.get_conditional_effects())
    {
        transform_conditions(effect->get_conditions<Static>(), m_negative_static_duals, transformed_action_static_conditions);
        transform_conditions(effect->get_conditions<Fluent>(), m_negative_fluent_duals, transformed_action_fluent_conditions);

        transform_fluent_effect_literal(effect->get_effect(), transformed_effect_literals);
        for (const auto& transformed_literal : transformed_effect_literals)
        {
            transformed_conditional_effects.push_back(
                this->m_pddl_factories.get_or_create_conditional_effect(transformed_action_static_conditions,
                                                                        transformed_action_fluent_conditions,
                                                                        this->transform(effect->get_conditions<Derived>()),
                                                                        transformed_literal));
        }
    }

    auto transformed_universal_effects = EffectUniversalList {};
    for (const auto& effect : action.get_universal_effects())
    {
        transform_conditions(effect->get_conditions<Static>(), m_negative_static_duals, transformed_action_static_conditions);
        transform_conditions(effect->get_conditions<Fluent>(), m_negative_fluent_duals, transformed_action_fluent_conditions);

        transform_fluent_effect_literal(effect->get_effect(), transformed_effect_literals);
        for (const auto& transformed_literal : transformed_effect_literals)
        {
            transformed_universal_effects.push_back(this->m_pddl_factories.get_or_create_universal_effect(this->transform(effect->get_parameters()),
                                                                                                          transformed_action_static_conditions,
                                                                                                          transformed_action_fluent_conditions,
                                                                                                          this->transform(effect->get_conditions<Derived>()),
                                                                                                          transformed_literal));
        }
    }

    return this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                       action.get_original_arity(),
                                                       this->transform(action.get_parameters()),
                                                       transformed_static_conditions,
                                                       transformed_fluent_conditions,
                                                       this->transform(action.get_conditions<Derived>()),
                                                       transformed_simple_effects,
                                                       transformed_conditional_effects,
                                                       transformed_universal_effects,
                                                       this->transform(*action.get_function_expression()));
}

template<PredicateCategory P>
void collect_positive_literals_by_atom(const LiteralSet<P>& literals, std::unordered_map<Atom<P>, Literal<P>>& out_positive_literals_by_atom)
{
    out_positive_literals_by_atom.clear();
    for (const auto& literal : literals)
    {
        if (!literal->is_negated())
        {
            out_positive_literals_by_atom.emplace(literal->get_atom(), literal);
        }
    }
}

Domain PositiveNormalFormTransformer::transform_impl(const DomainImpl& domain)
{
    auto transformed_static_predicates = this->transform(domain.get_predicates<Static>());
    auto transformed_fluent_predicates = this->transform(domain.get_predicates<Fluent>());

    auto positive_static_literals_by_atom = std::unordered_map<Atom<Static>, Literal<Static>> {};
    auto positive_fluent_literals_by_atom = std::unordered_map<Atom<Fluent>, Literal<Fluent>> {};
    collect_positive_literals_by_atom(m_static_literals, positive_static_literals_by_atom);
    collect_positive_literals_by_atom(m_fluent_literals, positive_fluent_literals_by_atom);

    compute_duals(domain,
                  m_negative_static_conditions,
                  positive_static_literals_by_atom,
                  m_positive_static_duals,
                  m_negative_static_duals,
                  m_static_dual_predicates);
    compute_duals(domain,
                  m_negative_fluent_conditions,
                  positive_fluent_literals_by_atom,
                  m_positive_fluent_duals,
                  m_negative_fluent_duals,
                  m_fluent_dual_predicates);

    // transformed_static_predicates.insert(transformed_static_predicates.end(), m_static_dual_predicates.begin(), m_static_dual_predicates.end());
    // transformed_fluent_predicates.insert(transformed_fluent_predicates.end(), m_fluent_dual_predicates.begin(), m_fluent_dual_predicates.end());

    return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                       this->transform(*domain.get_requirements()),
                                                       this->transform(domain.get_constants()),
                                                       transformed_static_predicates,
                                                       transformed_fluent_predicates,
                                                       this->transform(domain.get_predicates<Derived>()),
                                                       this->transform(domain.get_functions()),
                                                       this->transform(domain.get_actions()),
                                                       this->transform(domain.get_axioms()));
}

Problem PositiveNormalFormTransformer::transform_impl(const ProblemImpl& problem)
{
    // Transform domain first to obtain m_negative_fluent_duals
    const auto transformed_domain = this->transform(*problem.get_domain());

    auto transformed_static_initial_literals = this->transform(problem.get_static_initial_literals());
    auto transformed_fluent_initial_literals = this->transform(problem.get_fluent_initial_literals());

    auto additional_static_initial_literals = GroundLiteralList<Static> {};
    auto additional_fluent_initial_literals = GroundLiteralList<Fluent> {};

    // ISSUE: cannot create unknown opposite initial facts...
    // for (const auto& literal : transformed_static_initial_literals)
    //{
    //    const auto& predicate = literal->get_atom()->get_predicate();
    //    if (m_static_dual_predicates.count(literal->get_atom()->get_predicate()))
    //    {
    //        const auto dual_predicate = m_static_dual_predicates.at(predicate);
    //        additional_static_initial_literals.push_back(m_pddl_factories. m_pddl_factories.get_or_create_ground_atom(dual_predicate,
    //        literal->get_atom()->get_objects()))
    //    }
    //}

    transformed_static_initial_literals.insert(transformed_static_initial_literals.end(),
                                               additional_static_initial_literals.begin(),
                                               additional_static_initial_literals.end());
    transformed_fluent_initial_literals.insert(transformed_fluent_initial_literals.end(),
                                               additional_fluent_initial_literals.begin(),
                                               additional_fluent_initial_literals.end());

    return this->m_pddl_factories.get_or_create_problem(transformed_domain,
                                                        problem.get_name(),
                                                        this->transform(*problem.get_requirements()),
                                                        this->transform(problem.get_objects()),
                                                        this->transform(problem.get_derived_predicates()),
                                                        this->transform(problem.get_problem_and_domain_derived_predicates()),
                                                        transformed_static_initial_literals,
                                                        transformed_fluent_initial_literals,
                                                        this->transform(problem.get_numeric_fluents()),
                                                        this->transform(problem.get_goal_condition<Static>()),
                                                        this->transform(problem.get_goal_condition<Fluent>()),
                                                        this->transform(problem.get_goal_condition<Derived>()),
                                                        (problem.get_optimization_metric().has_value() ?
                                                             std::optional<OptimizationMetric>(this->transform(*problem.get_optimization_metric().value())) :
                                                             std::nullopt),
                                                        this->transform(problem.get_axioms()));
}

PositiveNormalFormTransformer::PositiveNormalFormTransformer(PDDLFactories& pddl_factories) :
    BaseCachedRecurseTransformer<PositiveNormalFormTransformer>(pddl_factories)
{
}
}
