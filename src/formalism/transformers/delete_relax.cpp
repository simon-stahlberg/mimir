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

#include "mimir/formalism/transformers/delete_relax.hpp"

#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

template<PredicateCategory P>
static LiteralList<P> filter_positive_literals(const LiteralList<P>& literals)
{
    auto positive_literals = LiteralList<P> {};
    for (const auto& literal : literals)
    {
        if (!literal->is_negated())
        {
            positive_literals.push_back(literal);
        }
    }
    return positive_literals;
}

LiteralList<Static> DeleteRelaxTransformer::transform_impl(const LiteralList<Static>& literals)
{
    auto positive_literals = LiteralList<Static> {};
    for (const auto& literal : literals)
    {
        const auto positive_literal = this->transform(*literal);
        if (positive_literal)
        {
            positive_literals.push_back(positive_literal);
        }
    }
    return positive_literals;
}

LiteralList<Fluent> DeleteRelaxTransformer::transform_impl(const LiteralList<Fluent>& literals)
{
    auto positive_literals = LiteralList<Fluent> {};
    for (const auto& literal : literals)
    {
        const auto positive_literal = this->transform(*literal);
        if (positive_literal)
        {
            positive_literals.push_back(positive_literal);
        }
    }
    return positive_literals;
}

LiteralList<Derived> DeleteRelaxTransformer::transform_impl(const LiteralList<Derived>& literals)
{
    auto positive_literals = LiteralList<Derived> {};
    for (const auto& literal : literals)
    {
        const auto positive_literal = this->transform(*literal);
        if (positive_literal)
        {
            positive_literals.push_back(positive_literal);
        }
    }
    return positive_literals;
}

EffectSimpleList DeleteRelaxTransformer::transform_impl(const EffectSimpleList& effects)
{
    auto positive_simple_effects = EffectSimpleList {};
    for (const auto& simple_effect : effects)
    {
        const auto positive_simple_effect = this->transform(*simple_effect);
        if (positive_simple_effect)
        {
            positive_simple_effects.push_back(positive_simple_effect);
        }
    }
    return positive_simple_effects;
}

EffectConditionalList DeleteRelaxTransformer::transform_impl(const EffectConditionalList& effects)
{
    auto positive_conditional_effects = EffectConditionalList {};
    for (const auto& conditional_effect : effects)
    {
        const auto positive_conditional_effect = this->transform(*conditional_effect);
        if (positive_conditional_effect)
        {
            positive_conditional_effects.push_back(positive_conditional_effect);
        }
    }
    return uniquify_elements(positive_conditional_effects);
}

EffectUniversalList DeleteRelaxTransformer::transform_impl(const EffectUniversalList& effects)
{
    auto positive_universal_effects = EffectUniversalList {};
    for (const auto& universal_effect : effects)
    {
        const auto positive_universal_effect = this->transform(*universal_effect);
        if (positive_universal_effect)
        {
            positive_universal_effects.push_back(positive_universal_effect);
        }
    }
    return uniquify_elements(positive_universal_effects);
}

ActionList DeleteRelaxTransformer::transform_impl(const ActionList& actions)
{
    auto relaxed_actions = ActionList {};
    for (const auto& action : actions)
    {
        const auto relaxed_action = this->transform(*action);
        if (relaxed_action)
        {
            relaxed_actions.push_back(relaxed_action);
        }
    }
    return uniquify_elements(relaxed_actions);
}

AxiomList DeleteRelaxTransformer::transform_impl(const AxiomList& axioms)
{
    auto relaxed_axioms = AxiomList {};
    for (const auto& axiom : axioms)
    {
        const auto relaxed_axiom = this->transform(*axiom);
        if (relaxed_axiom)
        {
            relaxed_axioms.push_back(relaxed_axiom);
        }
    }
    return uniquify_elements(relaxed_axioms);
}

Literal<Static> DeleteRelaxTransformer::transform_impl(const LiteralImpl<Static>& literal)
{
    if (literal.is_negated())
    {
        return nullptr;
    }

    const auto atom = this->transform(*literal.get_atom());

    return this->m_pddl_factories.get_or_create_literal(false, atom);
}

Literal<Fluent> DeleteRelaxTransformer::transform_impl(const LiteralImpl<Fluent>& literal)
{
    if (literal.is_negated())
    {
        return nullptr;
    }

    const auto atom = this->transform(*literal.get_atom());

    return this->m_pddl_factories.get_or_create_literal(false, atom);
}

Literal<Derived> DeleteRelaxTransformer::transform_impl(const LiteralImpl<Derived>& literal)
{
    if (literal.is_negated())
    {
        return nullptr;
    }

    const auto atom = this->transform(*literal.get_atom());

    return this->m_pddl_factories.get_or_create_literal(false, atom);
}

EffectSimple DeleteRelaxTransformer::transform_impl(const EffectSimpleImpl& effect)
{
    const auto literal = this->transform(*effect.get_effect());
    if (!literal)
    {
        return nullptr;
    }

    return this->m_pddl_factories.get_or_create_simple_effect(literal);
}

EffectConditional DeleteRelaxTransformer::transform_impl(const EffectConditionalImpl& effect)
{
    auto simple_effect = this->transform(*effect.get_effect());
    if (!simple_effect)
    {
        return nullptr;
    }

    auto static_conditions = filter_positive_literals(this->transform(effect.get_static_conditions()));
    auto fluent_conditions = filter_positive_literals(this->transform(effect.get_fluent_conditions()));
    auto derived_conditions = filter_positive_literals(this->transform(effect.get_derived_conditions()));

    return this->m_pddl_factories.get_or_create_conditional_effect(static_conditions, fluent_conditions, derived_conditions, simple_effect);
}

EffectUniversal DeleteRelaxTransformer::transform_impl(const EffectUniversalImpl& effect)
{
    auto simple_effect = this->transform(*effect.get_effect());
    if (!simple_effect)
    {
        return nullptr;
    }

    auto parameters = this->transform(effect.get_parameters());
    auto static_conditions = filter_positive_literals(this->transform(effect.get_static_conditions()));
    auto fluent_conditions = filter_positive_literals(this->transform(effect.get_fluent_conditions()));
    auto derived_conditions = filter_positive_literals(this->transform(effect.get_derived_conditions()));

    return this->m_pddl_factories.get_or_create_universal_effect(parameters, static_conditions, fluent_conditions, derived_conditions, simple_effect);
}

Action DeleteRelaxTransformer::transform_impl(const ActionImpl& action)
{
    auto simple_effects = this->transform(action.get_simple_effects());
    auto conditional_effects = this->transform(action.get_conditional_effects());
    auto universal_effects = this->transform(action.get_universal_effects());
    if (m_remove_useless_actions_and_axioms && simple_effects.empty() && conditional_effects.empty() && universal_effects.empty())
    {
        return nullptr;
    }

    auto parameters = this->transform(action.get_parameters());
    auto static_conditions = filter_positive_literals(this->transform(action.get_static_conditions()));
    auto fluent_conditions = filter_positive_literals(this->transform(action.get_fluent_conditions()));
    auto derived_conditions = filter_positive_literals(this->transform(action.get_derived_conditions()));
    auto cost_expression = this->transform(*action.get_function_expression());

    auto delete_relaxed_action = this->m_pddl_factories.get_or_create_action(action.get_name(),
                                                                             action.get_original_arity(),
                                                                             parameters,
                                                                             static_conditions,
                                                                             fluent_conditions,
                                                                             derived_conditions,
                                                                             simple_effects,
                                                                             conditional_effects,
                                                                             universal_effects,
                                                                             cost_expression);

    m_delete_to_normal_actions[delete_relaxed_action].push_back(&action);

    return delete_relaxed_action;
}

Axiom DeleteRelaxTransformer::transform_impl(const AxiomImpl& axiom)
{
    const auto literal = this->transform(*axiom.get_literal());
    auto parameters = this->transform(axiom.get_parameters());
    auto static_conditions = filter_positive_literals(this->transform(axiom.get_static_conditions()));
    auto fluent_conditions = filter_positive_literals(this->transform(axiom.get_fluent_conditions()));
    auto derived_conditions = filter_positive_literals(this->transform(axiom.get_derived_conditions()));

    auto delete_relaxed_axiom = this->m_pddl_factories.get_or_create_axiom(parameters, literal, static_conditions, fluent_conditions, derived_conditions);

    m_delete_to_normal_axioms[delete_relaxed_axiom].push_back(&axiom);

    return delete_relaxed_axiom;
}

Domain DeleteRelaxTransformer::transform_impl(const DomainImpl& domain)
{
    return this->m_pddl_factories.get_or_create_domain(domain.get_name(),
                                                       this->transform(*domain.get_requirements()),
                                                       this->transform(domain.get_constants()),
                                                       this->transform(domain.get_static_predicates()),
                                                       this->transform(domain.get_fluent_predicates()),
                                                       this->transform(domain.get_derived_predicates()),
                                                       this->transform(domain.get_functions()),
                                                       this->transform(domain.get_actions()),
                                                       this->transform(domain.get_axioms()));
}

Problem DeleteRelaxTransformer::run_impl(const ProblemImpl& problem)
{
    this->prepare(problem);
    return this->transform(problem);
}

DeleteRelaxTransformer::DeleteRelaxTransformer(PDDLFactories& pddl_factories, bool remove_useless_actions_and_axioms) :
    BaseCachedRecurseTransformer<DeleteRelaxTransformer>(pddl_factories),
    m_remove_useless_actions_and_axioms(remove_useless_actions_and_axioms)
{
}

const ActionList& DeleteRelaxTransformer::get_unrelaxed_actions(Action action) const { return m_delete_to_normal_actions.at(action); }

const AxiomList& DeleteRelaxTransformer::get_unrelaxed_axioms(Axiom axiom) const { return m_delete_to_normal_axioms.at(axiom); }

}
