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

void PositiveNormalFormTransformer::prepare_impl(const LiteralImpl<Fluent>& literal) { m_fluent_literals.insert(&literal); }

void PositiveNormalFormTransformer::prepare_impl(const EffectConditionalImpl& effect)
{
    for (const auto& literal : effect.get_conditions<Fluent>())
    {
        if (literal->is_negated())
        {
            m_negative_fluent_conditions.insert(literal);
        }
    }
}

void PositiveNormalFormTransformer::prepare_impl(const EffectUniversalImpl& effect)
{
    for (const auto& literal : effect.get_conditions<Fluent>())
    {
        if (literal->is_negated())
        {
            m_negative_fluent_conditions.insert(literal);
        }
    }
}

void PositiveNormalFormTransformer::prepare_impl(const ActionImpl& action)
{
    for (const auto& literal : action.get_conditions<Fluent>())
    {
        if (literal->is_negated())
        {
            m_negative_fluent_conditions.insert(literal);
        }
    }
}

Action PositiveNormalFormTransformer::transform_impl(const ActionImpl& action)
{
    auto transformed_fluent_conditions = LiteralList<Fluent> {};
    transform_conditions(action.get_conditions<Fluent>(), m_negative_duals, transformed_fluent_conditions);

    const auto transform_fluent_effect_literal = [&, this](const Literal<Fluent>& literal)
    {
        const auto transformed_literal = this->transform(*literal);
        if (this->m_negative_duals.count(literal))
        {
            return LiteralList<Fluent> { transformed_literal, this->m_negative_duals.at(literal) };
        }
        else if (this->m_positive_duals.count(literal))
        {
            return LiteralList<Fluent> { transformed_literal, this->m_positive_duals.at(literal) };
        }
        else
        {
            return LiteralList<Fluent> { transformed_literal };
        }
    };

    auto transformed_simple_effects = EffectSimpleList {};
    for (const auto& effect : action.get_simple_effects())
    {
        const auto transformed_fluent_effect_literals = transform_fluent_effect_literal(effect->get_effect());
        for (const auto& transformed_literal : transformed_fluent_effect_literals)
        {
            transformed_simple_effects.push_back(this->m_pddl_factories.get_or_create_simple_effect(transformed_literal));
        }
    }

    auto transformed_conditional_effects = EffectConditionalList {};
    auto transformed_action_fluent_conditions = LiteralList<Fluent> {};
    for (const auto& effect : action.get_conditional_effects())
    {
        transform_conditions(effect->get_conditions<Fluent>(), m_negative_duals, transformed_action_fluent_conditions);
        const auto transformed_fluent_effect_literals = transform_fluent_effect_literal(effect->get_effect());
        for (const auto& transformed_literal : transformed_fluent_effect_literals)
        {
            // TODO: static and derived conditions..
            transformed_conditional_effects.push_back(
                this->m_pddl_factories.get_or_create_conditional_effect(this->transform(effect->get_conditions<Static>()),
                                                                        transformed_action_fluent_conditions,
                                                                        this->transform(effect->get_conditions<Derived>()),
                                                                        transformed_literal));
        }
    }

    auto transformed_universal_effects = EffectUniversalList {};
    for (const auto& effect : action.get_universal_effects())
    {
        transform_conditions(effect->get_conditions<Fluent>(), m_negative_duals, transformed_action_fluent_conditions);
        const auto transformed_fluent_effect_literals = transform_fluent_effect_literal(effect->get_effect());
        for (const auto& transformed_literal : transformed_fluent_effect_literals)
        {
            // TODO: static and derived conditions..
            transformed_universal_effects.push_back(this->m_pddl_factories.get_or_create_universal_effect(this->transform(effect->get_parameters()),
                                                                                                          this->transform(effect->get_conditions<Static>()),
                                                                                                          transformed_action_fluent_conditions,
                                                                                                          this->transform(effect->get_conditions<Derived>()),
                                                                                                          transformed_literal));
        }
    }

    return this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                       action.get_original_arity(),
                                                       this->transform(action.get_parameters()),
                                                       this->transform(action.get_conditions<Static>()),
                                                       transformed_fluent_conditions,
                                                       this->transform(action.get_conditions<Derived>()),
                                                       transformed_simple_effects,
                                                       transformed_conditional_effects,
                                                       transformed_universal_effects,
                                                       this->transform(*action.get_function_expression()));
}

Domain PositiveNormalFormTransformer::transform_impl(const DomainImpl& domain)
{
    auto translated_fluent_predicates = this->transform(domain.get_predicates<Fluent>());

    auto positive_literals_by_atom = std::unordered_map<Atom<Fluent>, Literal<Fluent>> {};
    for (const auto& literal : m_fluent_literals)
    {
        if (!literal->is_negated())
        {
            positive_literals_by_atom.emplace(literal->get_atom(), literal);
        }
    }

    for (const auto& negative_literal : m_negative_fluent_conditions)
    {
        const auto& predicate = negative_literal->get_atom()->get_predicate();
        const auto dual_predicate = m_pddl_factories.get_or_create_predicate<Fluent>("not " + predicate->get_name(), predicate->get_parameters());
        // TODO: throw exception if dual_predicate already existed before.
        translated_fluent_predicates.push_back(dual_predicate);

        const auto positive_literal = positive_literals_by_atom.at(negative_literal->get_atom());
        const auto dual_for_positive_literal =
            m_pddl_factories.get_or_create_literal(true, m_pddl_factories.get_or_create_atom(dual_predicate, positive_literal->get_atom()->get_terms()));
        m_positive_duals.emplace(positive_literal, dual_for_positive_literal);

        const auto dual_for_negative_literal =
            m_pddl_factories.get_or_create_literal(false, m_pddl_factories.get_or_create_atom(dual_predicate, negative_literal->get_atom()->get_terms()));
        m_negative_duals.emplace(negative_literal, dual_for_negative_literal);
    }

    return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                       this->transform(*domain.get_requirements()),
                                                       this->transform(domain.get_constants()),
                                                       this->transform(domain.get_predicates<Static>()),
                                                       translated_fluent_predicates,
                                                       this->transform(domain.get_predicates<Derived>()),
                                                       this->transform(domain.get_functions()),
                                                       this->transform(domain.get_actions()),
                                                       this->transform(domain.get_axioms()));
}

PositiveNormalFormTransformer::PositiveNormalFormTransformer(PDDLFactories& pddl_factories) :
    BaseCachedRecurseTransformer<PositiveNormalFormTransformer>(pddl_factories)
{
}
}
