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

template<PredicateTag P>
LiteralList<P> DeleteRelaxTransformer::transform_impl(const LiteralList<P>& literals)
{
    auto positive_literals = LiteralList<P> {};
    for (const auto& literal : literals)
    {
        const auto positive_literal = this->transform(literal);
        if (positive_literal)
        {
            positive_literals.push_back(positive_literal);
        }
    }
    return positive_literals;
}

template LiteralList<Static> DeleteRelaxTransformer::transform_impl(const LiteralList<Static>& literals);
template LiteralList<Fluent> DeleteRelaxTransformer::transform_impl(const LiteralList<Fluent>& literals);
template LiteralList<Derived> DeleteRelaxTransformer::transform_impl(const LiteralList<Derived>& literals);

ConditionalEffectList DeleteRelaxTransformer::transform_impl(const ConditionalEffectList& effects)
{
    auto positive_conditional_effects = ConditionalEffectList {};
    for (const auto& conditional_effect : effects)
    {
        positive_conditional_effects.push_back(this->transform(conditional_effect));
    }
    return uniquify_elements(positive_conditional_effects);
}

ActionList DeleteRelaxTransformer::transform_impl(const ActionList& actions)
{
    auto relaxed_actions = ActionList {};
    for (const auto& action : actions)
    {
        relaxed_actions.push_back(this->transform(action));
    }
    return uniquify_elements(relaxed_actions);
}

AxiomList DeleteRelaxTransformer::transform_impl(const AxiomList& axioms)
{
    auto relaxed_axioms = AxiomList {};
    for (const auto& axiom : axioms)
    {
        relaxed_axioms.push_back(this->transform(axiom));
    }
    return uniquify_elements(relaxed_axioms);
}

template<PredicateTag P>
Literal<P> DeleteRelaxTransformer::transform_impl(Literal<P> literal)
{
    if (literal->is_negated())
    {
        return nullptr;
    }

    const auto atom = this->transform(literal->get_atom());

    return this->m_pddl_repositories.get_or_create_literal(false, atom);
}

template Literal<Static> DeleteRelaxTransformer::transform_impl(Literal<Static> literal);
template Literal<Fluent> DeleteRelaxTransformer::transform_impl(Literal<Fluent> literal);
template Literal<Derived> DeleteRelaxTransformer::transform_impl(Literal<Derived> literal);

ConjunctiveCondition DeleteRelaxTransformer::transform_impl(ConjunctiveCondition condition)
{
    auto parameters = this->transform(condition->get_parameters());
    auto static_literals = filter_positive_literals(this->transform(condition->get_literals<Static>()));
    auto fluent_literals = filter_positive_literals(this->transform(condition->get_literals<Fluent>()));
    auto derived_literals = filter_positive_literals(this->transform(condition->get_literals<Derived>()));
    auto numeric_constraints = NumericConstraintList {};

    return this->m_pddl_repositories.get_or_create_conjunctive_condition(std::move(parameters),
                                                                         std::move(static_literals),
                                                                         std::move(fluent_literals),
                                                                         std::move(derived_literals),
                                                                         std::move(numeric_constraints));
}

ConjunctiveEffect DeleteRelaxTransformer::transform_impl(ConjunctiveEffect effect)
{
    auto fluent_literals = filter_positive_literals(this->transform(effect->get_literals()));
    auto fluent_numeric_effects = NumericEffectList<Fluent> {};
    auto auxiliary_numeric_effects = NumericEffectList<Auxiliary> {};

    return this->m_pddl_repositories.get_or_create_conjunctive_effect(effect->get_parameters(),
                                                                      std::move(fluent_literals),
                                                                      std::move(fluent_numeric_effects),
                                                                      std::move(auxiliary_numeric_effects));
}

ConditionalEffect DeleteRelaxTransformer::transform_impl(ConditionalEffect effect)
{
    auto transformed_conjunctive_condition = this->transform(effect->get_conjunctive_condition());
    auto transformed_conjunctive_effect = this->transform(effect->get_conjunctive_effect());

    return this->m_pddl_repositories.get_or_create_conditional_effect(transformed_conjunctive_condition, transformed_conjunctive_effect);
}

Action DeleteRelaxTransformer::transform_impl(Action action)
{
    auto conjunctive_effect = this->transform(action->get_conjunctive_effect());
    auto conditional_effects = this->transform(action->get_conditional_effects());

    auto conjunctive_condition = this->transform(action->get_conjunctive_condition());

    auto delete_relaxed_action = this->m_pddl_repositories.get_or_create_action(action->get_name(),
                                                                                action->get_original_arity(),
                                                                                conjunctive_condition,
                                                                                conjunctive_effect,
                                                                                conditional_effects);

    m_delete_to_normal_actions[delete_relaxed_action].push_back(action);

    return delete_relaxed_action;
}

Axiom DeleteRelaxTransformer::transform_impl(Axiom axiom)
{
    auto conjunctive_condition = this->transform(axiom->get_conjunctive_condition());
    const auto literal = this->transform(axiom->get_literal());

    auto delete_relaxed_axiom = this->m_pddl_repositories.get_or_create_axiom(conjunctive_condition, literal);

    m_delete_to_normal_axioms[delete_relaxed_axiom].push_back(axiom);

    return delete_relaxed_axiom;
}

Problem DeleteRelaxTransformer::run_impl(Problem problem)
{
    this->prepare(problem);
    return this->transform(problem);
}

DeleteRelaxTransformer::DeleteRelaxTransformer(PDDLRepositories& pddl_repositories) : BaseCachedRecurseTransformer<DeleteRelaxTransformer>(pddl_repositories) {}

const ActionList& DeleteRelaxTransformer::get_unrelaxed_actions(Action action) const { return m_delete_to_normal_actions.at(action); }

const AxiomList& DeleteRelaxTransformer::get_unrelaxed_axioms(Axiom axiom) const { return m_delete_to_normal_axioms.at(axiom); }

}
