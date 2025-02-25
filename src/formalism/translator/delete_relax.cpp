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

#include "mimir/formalism/translator/delete_relax.hpp"

#include "mimir/common/collections.hpp"

namespace mimir
{

template<StaticOrFluentOrDerived P>
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

template<StaticOrFluentOrDerived P>
LiteralList<P> DeleteRelaxTranslator::translate_level_2(const LiteralList<P>& literals, PDDLRepositories& repositories)
{
    auto positive_literals = LiteralList<P> {};
    for (const auto& literal : literals)
    {
        const auto positive_literal = this->translate_level_0(literal, repositories);
        if (positive_literal)
        {
            positive_literals.push_back(positive_literal);
        }
    }
    return positive_literals;
}

template LiteralList<Static> DeleteRelaxTranslator::translate_level_2(const LiteralList<Static>& literals, PDDLRepositories& repositories);
template LiteralList<Fluent> DeleteRelaxTranslator::translate_level_2(const LiteralList<Fluent>& literals, PDDLRepositories& repositories);
template LiteralList<Derived> DeleteRelaxTranslator::translate_level_2(const LiteralList<Derived>& literals, PDDLRepositories& repositories);

ConditionalEffectList DeleteRelaxTranslator::translate_level_2(const ConditionalEffectList& effects, PDDLRepositories& repositories)
{
    auto positive_conditional_effects = ConditionalEffectList {};
    for (const auto& conditional_effect : effects)
    {
        positive_conditional_effects.push_back(this->translate_level_0(conditional_effect, repositories));
    }
    return uniquify_elements(positive_conditional_effects);
}

ActionList DeleteRelaxTranslator::translate_level_2(const ActionList& actions, PDDLRepositories& repositories)
{
    auto relaxed_actions = ActionList {};
    for (const auto& action : actions)
    {
        relaxed_actions.push_back(this->translate_level_0(action, repositories));
    }
    return uniquify_elements(relaxed_actions);
}

AxiomList DeleteRelaxTranslator::translate_level_2(const AxiomList& axioms, PDDLRepositories& repositories)
{
    auto relaxed_axioms = AxiomList {};
    for (const auto& axiom : axioms)
    {
        relaxed_axioms.push_back(this->translate_level_0(axiom, repositories));
    }
    return uniquify_elements(relaxed_axioms);
}

template<StaticOrFluentOrDerived P>
Literal<P> DeleteRelaxTranslator::translate_level_2(Literal<P> literal, PDDLRepositories& repositories)
{
    if (literal->is_negated())
    {
        return nullptr;
    }

    const auto atom = this->translate_level_0(literal->get_atom(), repositories);

    return repositories.get_or_create_literal(false, atom);
}

template Literal<Static> DeleteRelaxTranslator::translate_level_2(Literal<Static> literal, PDDLRepositories& repositories);
template Literal<Fluent> DeleteRelaxTranslator::translate_level_2(Literal<Fluent> literal, PDDLRepositories& repositories);
template Literal<Derived> DeleteRelaxTranslator::translate_level_2(Literal<Derived> literal, PDDLRepositories& repositories);

ConjunctiveCondition DeleteRelaxTranslator::translate_level_2(ConjunctiveCondition condition, PDDLRepositories& repositories)
{
    auto translated_parameters = this->translate_level_0(condition->get_parameters(), repositories);
    auto translated_literals = LiteralLists<Static, Fluent, Derived> {};
    boost::hana::for_each(condition->get_hana_literals(),
                          [&](auto&& pair)
                          {
                              const auto& key = boost::hana::first(pair);
                              const auto& value = boost::hana::second(pair);

                              boost::hana::at_key(translated_literals, key) = filter_positive_literals(self().translate_level_0(value, repositories));
                          });

    auto numeric_constraints = NumericConstraintList {};

    return repositories.get_or_create_conjunctive_condition(std::move(translated_parameters), std::move(translated_literals), std::move(numeric_constraints));
}

ConjunctiveEffect DeleteRelaxTranslator::translate_level_2(ConjunctiveEffect effect, PDDLRepositories& repositories)
{
    auto translated_fluent_literals = filter_positive_literals(this->translate_level_0(effect->get_literals(), repositories));
    auto translated_fluent_numeric_effects = NumericEffectList<Fluent> {};
    auto translated_auxiliary_numeric_effect = std::optional<NumericEffect<Auxiliary>> { std::nullopt };

    return repositories.get_or_create_conjunctive_effect(effect->get_parameters(),
                                                         std::move(translated_fluent_literals),
                                                         std::move(translated_fluent_numeric_effects),
                                                         std::move(translated_auxiliary_numeric_effect));
}

ConditionalEffect DeleteRelaxTranslator::translate_level_2(ConditionalEffect effect, PDDLRepositories& repositories)
{
    auto translated__conjunctive_condition = this->translate_level_0(effect->get_conjunctive_condition(), repositories);
    auto translated__conjunctive_effect = this->translate_level_0(effect->get_conjunctive_effect(), repositories);

    return repositories.get_or_create_conditional_effect(translated__conjunctive_condition, translated__conjunctive_effect);
}

Action DeleteRelaxTranslator::translate_level_2(Action action, PDDLRepositories& repositories)
{
    auto translated_conjunctive_effect = this->translate_level_0(action->get_conjunctive_effect(), repositories);
    auto translated_conditional_effects = this->translate_level_0(action->get_conditional_effects(), repositories);

    auto translated_conjunctive_condition = this->translate_level_0(action->get_conjunctive_condition(), repositories);

    auto delete_relaxed_action = repositories.get_or_create_action(action->get_name(),
                                                                   action->get_original_arity(),
                                                                   translated_conjunctive_condition,
                                                                   translated_conjunctive_effect,
                                                                   translated_conditional_effects);

    m_delete_to_normal_actions[delete_relaxed_action].push_back(action);

    return delete_relaxed_action;
}

Axiom DeleteRelaxTranslator::translate_level_2(Axiom axiom, PDDLRepositories& repositories)
{
    auto translated_conjunctive_condition = this->translate_level_0(axiom->get_conjunctive_condition(), repositories);
    auto translated_literal = this->translate_level_0(axiom->get_literal(), repositories);

    auto delete_relaxed_axiom = repositories.get_or_create_axiom(translated_conjunctive_condition, translated_literal);

    m_delete_to_normal_axioms[delete_relaxed_axiom].push_back(axiom);

    return delete_relaxed_axiom;
}

const ActionList& DeleteRelaxTranslator::get_unrelaxed_actions(Action action) const { return m_delete_to_normal_actions.at(action); }

const AxiomList& DeleteRelaxTranslator::get_unrelaxed_axioms(Axiom axiom) const { return m_delete_to_normal_axioms.at(axiom); }

}
