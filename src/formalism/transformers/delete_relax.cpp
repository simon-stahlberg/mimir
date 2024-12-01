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

#include "mimir/common/collections.hpp"
#include "mimir/formalism/translators/utils.hpp"

namespace mimir
{

template<PredicateTag P>
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

EffectStrips DeleteRelaxTransformer::transform_impl(const EffectStripsImpl& effect)
{
    return this->m_pddl_repositories.get_or_create_strips_effect(this->transform(effect.get_effects()), this->transform(*effect.get_function_expression()));
}

EffectConditional DeleteRelaxTransformer::transform_impl(const EffectConditionalImpl& effect)
{
    auto transformed_literals = LiteralList<Fluent> {};
    for (const auto& literal : this->transform(effect.get_effects()))
    {
        if (literal)
        {
            transformed_literals.push_back(literal);
        }
    }
    if (transformed_literals.empty())
    {
        return nullptr;
    }

    auto parameters = this->transform(effect.get_parameters());
    auto static_conditions = filter_positive_literals(this->transform(effect.get_conditions<Static>()));
    auto fluent_conditions = filter_positive_literals(this->transform(effect.get_conditions<Fluent>()));
    auto derived_conditions = filter_positive_literals(this->transform(effect.get_conditions<Derived>()));
    auto function_expression = this->transform(*effect.get_function_expression());

    return this->m_pddl_repositories
        .get_or_create_conditional_effect(parameters, static_conditions, fluent_conditions, derived_conditions, transformed_literals, function_expression);
}

Action DeleteRelaxTransformer::transform_impl(const ActionImpl& action)
{
    auto strips_effect = this->transform(*action.get_strips_effect());
    auto conditional_effects = this->transform(action.get_conditional_effects());

    if (m_remove_useless_actions_and_axioms && strips_effect->get_effects().empty() && conditional_effects.empty())
    {
        return nullptr;
    }

    auto parameters = this->transform(action.get_parameters());
    auto static_conditions = filter_positive_literals(this->transform(action.get_conditions<Static>()));
    auto fluent_conditions = filter_positive_literals(this->transform(action.get_conditions<Fluent>()));
    auto derived_conditions = filter_positive_literals(this->transform(action.get_conditions<Derived>()));

    auto delete_relaxed_action = this->m_pddl_repositories.get_or_create_action(action.get_name(),
                                                                                action.get_original_arity(),
                                                                                parameters,
                                                                                static_conditions,
                                                                                fluent_conditions,
                                                                                derived_conditions,
                                                                                strips_effect,
                                                                                conditional_effects);

    m_delete_to_normal_actions[delete_relaxed_action].push_back(&action);

    return delete_relaxed_action;
}

Axiom DeleteRelaxTransformer::transform_impl(const AxiomImpl& axiom)
{
    const auto literal = this->transform(*axiom.get_literal());
    auto parameters = this->transform(axiom.get_parameters());
    auto static_conditions = filter_positive_literals(this->transform(axiom.get_conditions<Static>()));
    auto fluent_conditions = filter_positive_literals(this->transform(axiom.get_conditions<Fluent>()));
    auto derived_conditions = filter_positive_literals(this->transform(axiom.get_conditions<Derived>()));

    auto delete_relaxed_axiom = this->m_pddl_repositories.get_or_create_axiom(parameters, literal, static_conditions, fluent_conditions, derived_conditions);

    m_delete_to_normal_axioms[delete_relaxed_axiom].push_back(&axiom);

    return delete_relaxed_axiom;
}

Domain DeleteRelaxTransformer::transform_impl(const DomainImpl& domain)
{
    auto transformed_domain = this->m_pddl_repositories.get_or_create_domain(domain.get_filepath(),
                                                                             domain.get_name(),
                                                                             this->transform(*domain.get_requirements()),
                                                                             this->transform(domain.get_constants()),
                                                                             this->transform(domain.get_predicates<Static>()),
                                                                             this->transform(domain.get_predicates<Fluent>()),
                                                                             this->transform(domain.get_predicates<Derived>()),
                                                                             this->transform(domain.get_functions()),
                                                                             this->transform(domain.get_actions()),
                                                                             this->transform(domain.get_axioms()));

    return transformed_domain;
}

Problem DeleteRelaxTransformer::run_impl(const ProblemImpl& problem)
{
    this->prepare(problem);
    return this->transform(problem);
}

DeleteRelaxTransformer::DeleteRelaxTransformer(PDDLRepositories& pddl_repositories, bool remove_useless_actions_and_axioms) :
    BaseCachedRecurseTransformer<DeleteRelaxTransformer>(pddl_repositories),
    m_remove_useless_actions_and_axioms(remove_useless_actions_and_axioms)
{
}

const ActionList& DeleteRelaxTransformer::get_unrelaxed_actions(Action action) const { return m_delete_to_normal_actions.at(action); }

const AxiomList& DeleteRelaxTransformer::get_unrelaxed_axioms(Axiom axiom) const { return m_delete_to_normal_axioms.at(axiom); }

}
