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

/**
 * Prepare step.
 */

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

void PositiveNormalFormTransformer::prepare_impl(const LiteralImpl<Derived>& literal) { m_derived_literals.insert(&literal); }

void PositiveNormalFormTransformer::prepare_impl(const EffectConditionalImpl& effect)
{
    collect_negative_conditions(effect.get_conditions<Static>(), m_negative_static_conditions);
    collect_negative_conditions(effect.get_conditions<Fluent>(), m_negative_fluent_conditions);
    collect_negative_conditions(effect.get_conditions<Derived>(), m_negative_derived_conditions);
}

void PositiveNormalFormTransformer::prepare_impl(const EffectUniversalImpl& effect)
{
    collect_negative_conditions(effect.get_conditions<Static>(), m_negative_static_conditions);
    collect_negative_conditions(effect.get_conditions<Fluent>(), m_negative_fluent_conditions);
    collect_negative_conditions(effect.get_conditions<Derived>(), m_negative_derived_conditions);
}

void PositiveNormalFormTransformer::prepare_impl(const ActionImpl& action)
{
    collect_negative_conditions(action.get_conditions<Static>(), m_negative_static_conditions);
    collect_negative_conditions(action.get_conditions<Fluent>(), m_negative_fluent_conditions);
    collect_negative_conditions(action.get_conditions<Derived>(), m_negative_derived_conditions);
}

/**
 * Transform step
 */

EffectConditional PositiveNormalFormTransformer::transform_impl(const EffectConditionalImpl& effect)
{
    auto transformed_static_conditions = this->transform(effect.get_conditions<Static>());
    auto transformed_fluent_conditions = this->transform(effect.get_conditions<Fluent>());
    auto transformed_derived_conditions = this->transform(effect.get_conditions<Derived>());

    transform_conditions(effect.get_conditions<Static>(), m_negative_static_duals, transformed_static_conditions, transformed_derived_conditions);
    transform_conditions(effect.get_conditions<Fluent>(), m_negative_fluent_duals, transformed_fluent_conditions, transformed_derived_conditions);
    transform_conditions(effect.get_conditions<Derived>(), m_negative_derived_duals, transformed_derived_conditions, transformed_derived_conditions);

    return this->m_pddl_factories.get_or_create_conditional_effect(transformed_static_conditions,
                                                                   transformed_fluent_conditions,
                                                                   transformed_derived_conditions,
                                                                   this->transform(*effect.get_effect()));
}

EffectUniversal PositiveNormalFormTransformer::transform_impl(const EffectUniversalImpl& effect)
{
    auto transformed_static_conditions = this->transform(effect.get_conditions<Static>());
    auto transformed_fluent_conditions = this->transform(effect.get_conditions<Fluent>());
    auto transformed_derived_conditions = this->transform(effect.get_conditions<Derived>());

    transform_conditions(effect.get_conditions<Static>(), m_negative_static_duals, transformed_static_conditions, transformed_derived_conditions);
    transform_conditions(effect.get_conditions<Fluent>(), m_negative_fluent_duals, transformed_fluent_conditions, transformed_derived_conditions);
    transform_conditions(effect.get_conditions<Derived>(), m_negative_derived_duals, transformed_derived_conditions, transformed_derived_conditions);
}

Action PositiveNormalFormTransformer::transform_impl(const ActionImpl& action)
{
    auto transformed_static_conditions = this->transform(action.get_conditions<Static>());
    auto transformed_fluent_conditions = this->transform(action.get_conditions<Fluent>());
    auto transformed_derived_conditions = this->transform(action.get_conditions<Derived>());

    transform_conditions(action.get_conditions<Static>(), m_negative_static_duals, transformed_static_conditions, transformed_derived_conditions);
    transform_conditions(action.get_conditions<Fluent>(), m_negative_fluent_duals, transformed_fluent_conditions, transformed_derived_conditions);
    transform_conditions(action.get_conditions<Derived>(), m_negative_derived_duals, transformed_derived_conditions, transformed_derived_conditions);

    return this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                       action.get_original_arity(),
                                                       this->transform(action.get_parameters()),
                                                       transformed_static_conditions,
                                                       transformed_fluent_conditions,
                                                       transformed_derived_conditions,
                                                       this->transform(action.get_simple_effects()),
                                                       this->transform(action.get_conditional_effects()),
                                                       this->transform(action.get_universal_effects()),
                                                       this->transform(*action.get_function_expression()));
}

Domain PositiveNormalFormTransformer::transform_impl(const DomainImpl& domain)
{
    auto transformed_derived_predicates = this->transform(domain.get_predicates<Derived>());
    compute_duals(domain, m_negative_static_conditions, m_negative_static_duals, transformed_derived_predicates);
    compute_duals(domain, m_negative_fluent_conditions, m_negative_fluent_duals, transformed_derived_predicates);
    compute_duals(domain, m_negative_derived_conditions, m_negative_derived_duals, transformed_derived_predicates);

    // Note: Keep negative axiom conditions, since axioms applicability remains identical.
    // Also, we are introducing axioms of form â :- not a, which would contradict a transformation of negative conditions.

    auto transformed_axioms = this->transform(domain.get_axioms());
    introduce_axiom_for_dual(m_negative_static_duals, transformed_axioms);
    introduce_axiom_for_dual(m_negative_fluent_duals, transformed_axioms);
    introduce_axiom_for_dual(m_negative_derived_duals, transformed_axioms);

    return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                       this->transform(*domain.get_requirements()),
                                                       this->transform(domain.get_constants()),
                                                       this->transform(domain.get_predicates<Static>()),
                                                       this->transform(domain.get_predicates<Fluent>()),
                                                       transformed_derived_predicates,
                                                       this->transform(domain.get_functions()),
                                                       this->transform(domain.get_actions()),
                                                       transformed_axioms);
}

PositiveNormalFormTransformer::PositiveNormalFormTransformer(PDDLFactories& pddl_factories) :
    BaseCachedRecurseTransformer<PositiveNormalFormTransformer>(pddl_factories)
{
}
}
