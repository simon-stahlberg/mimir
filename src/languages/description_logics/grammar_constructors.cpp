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

#include "mimir/languages/description_logics/visitors.hpp"

#include <mimir/languages/description_logics/grammar_constructors.hpp>

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
NonTerminal<D>::NonTerminal(size_t id, std::string name, std::unique_ptr<const DerivationRule<D>*>&& rule) :
    m_id(id),
    m_name(std::move(name)),
    m_rule(std::move(rule))
{
}

template<dl::IsConceptOrRole D>
bool NonTerminal<D>::operator==(const NonTerminal& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name);
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t NonTerminal<D>::hash() const
{
    return std::hash<std::string>()(m_name);
}

template<dl::IsConceptOrRole D>
bool NonTerminal<D>::test_match(const D& constructor) const
{
    return (*m_rule)->test_match(constructor);
}

template<dl::IsConceptOrRole D>
size_t NonTerminal<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const DerivationRule<D>& NonTerminal<D>::get_rule() const
{
    return *m_rule.get();
}

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
DerivationRule<D>::DerivationRule(size_t id, std::vector<Choice<D>> choices) : m_id(id), m_choices(std::move(choices))
{
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::operator==(const DerivationRule& other) const
{
    if (this != &other)
    {
        return (m_choices == other.m_choices);
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::hash() const
{
    size_t seed = m_choices.size();
    std::for_each(m_choices.begin(),
                  m_choices.end(),
                  [&seed](const auto& choice) { loki::hash_combine(seed, std::visit([](const auto& arg) { return arg->hash(); })); });
    return seed;
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::test_match(const D& constructor) const
{
    return std::any_of(m_choices.begin(),
                       m_choices.end(),
                       [&constructor](const Choice<D>& choice)
                       { return std::visit([&constructor](const auto& arg) -> bool { return arg->test_match(constructor); }, choice); });
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::get_id() const
{
    return m_id;
}

/**
 * ConceptPredicateState
 */

template<PredicateCategory P>
ConceptPredicateState<P>::ConceptPredicateState(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::operator==(const ConceptPredicateState& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::test_match(const dl::Concept& constructor) const
{
    return constructor.accept(ConceptPredicateStateVisitor(this));
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateState<P>::get_predicate() const
{
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::get_id() const
{
    return m_id;
}

/**
 * ConceptPredicateGoal
 */

template<PredicateCategory P>
ConceptPredicateGoal<P>::ConceptPredicateGoal(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::operator==(const ConceptPredicateGoal& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::test_match(const dl::Concept& constructor) const
{
    return constructor.accept(ConceptPredicateGoalVisitor(this));
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateGoal<P>::get_predicate() const
{
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::get_id() const
{
    return m_id;
}

/**
 * ConceptAnd
 */
ConceptAnd::ConceptAnd(size_t id, ConceptChoice concept_left, ConceptChoice concept_right) :
    m_id(id),
    m_concept_left(std::move(concept_left)),
    m_concept_right(std::move(concept_right))
{
}

bool ConceptAnd::operator==(const ConceptAnd& other) const
{
    if (this != &other)
    {
        return (m_concept_left == other.m_concept_left) && (m_concept_right == other.m_concept_right);
    }
    return true;
}

size_t ConceptAnd::hash() const { return loki::hash_combine(m_concept_left, m_concept_right); }

bool ConceptAnd::test_match(const dl::Concept& constructor) const { return constructor.accept(ConceptAndVisitor(*this)); }

const ConceptChoice& ConceptAnd::get_concept_left() const { return m_concept_left; }

const ConceptChoice& ConceptAnd::get_concept_right() const { return m_concept_right; }

size_t ConceptAnd::get_id() const { return m_id; }

/**
 * RolePredicateState
 */

template<PredicateCategory P>
RolePredicateState<P>::RolePredicateState(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateState<P>::operator==(const RolePredicateState& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
size_t RolePredicateState<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool RolePredicateState<P>::test_match(const dl::Role& constructor) const
{
    return constructor.accept(RolePredicateStateVisitor(this));
}

template<PredicateCategory P>
Predicate<P> RolePredicateState<P>::get_predicate() const
{
}

template<PredicateCategory P>
size_t RolePredicateState<P>::get_id() const
{
    return m_id;
}

/**
 * RolePredicateGoal
 */

template<PredicateCategory P>
RolePredicateGoal<P>::RolePredicateGoal(size_t id, Predicate<P> predicate) : m_id(id), m_predicate(predicate)
{
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::operator==(const RolePredicateGoal& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate);
    }
    return true;
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::test_match(const dl::Role& constructor) const
{
    return constructor.accept(RolePredicateGoalVisitor(this));
}

template<PredicateCategory P>
Predicate<P> RolePredicateGoal<P>::get_predicate() const
{
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::get_id() const
{
    return m_id;
}

/**
 * RoleAnd
 */
RoleAnd::RoleAnd(size_t id, const Role& role_left, const Role& role_right) : m_id(id), m_role_left(&role_left), m_role_right(&role_right) {}

bool RoleAnd::operator==(const RoleAnd& other) const
{
    if (this != &other)
    {
        return (m_role_left == other.m_role_left) && (m_role_right == other.m_role_right);
    }
    return true;
}

size_t RoleAnd::hash() const { return loki::hash_combine(m_role_left, m_role_right); }

bool RoleAnd::test_match(const dl::Role& constructor) const { return constructor.accept(RoleAndVisitor(*this)); }

const Role& RoleAnd::get_role_left() const { return *m_role_left; }

const Role& RoleAnd::get_role_right() const { return *m_role_right; }

size_t RoleAnd::get_id() const { return m_id; }

}
