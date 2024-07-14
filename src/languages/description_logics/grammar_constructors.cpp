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

#include <mimir/languages/description_logics/grammar_constructors.hpp>
#include <mimir/languages/description_logics/grammar_visitors.hpp>

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
NonTerminal<D>::NonTerminal(size_t id, std::string name) : m_id(id), m_name(std::move(name)), m_rule(std::nullopt)
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
bool NonTerminal<D>::test_match(const dl::Constructor<D>& constructor) const
{
    assert(m_rule.has_value());
    return m_rule.value().get().test_match(constructor);
}

template<dl::IsConceptOrRole D>
size_t NonTerminal<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const std::string& NonTerminal<D>::get_name() const
{
    return m_name;
}

template<dl::IsConceptOrRole D>
const DerivationRule<D>& NonTerminal<D>::get_rule() const
{
    assert(m_rule.has_value());
    return m_rule.value();
}

template<dl::IsConceptOrRole D>
void NonTerminal<D>::set_rule(const DerivationRule<D>& rule) const
{
    m_rule = rule;
}

template class NonTerminal<Concept>;
template class NonTerminal<Role>;

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
Choice<D>::Choice(size_t id, ConstructorOrNonTerminalChoice<D> choice) : m_id(id), m_choice(std::move(choice))
{
}

template<dl::IsConceptOrRole D>
bool Choice<D>::operator==(const Choice& other) const
{
    if (this != &other)
    {
        return std::visit(
            [](const auto& lhs, const auto& rhs) -> bool
            {
                using LHS = std::decay_t<decltype(lhs)>;
                using RHS = std::decay_t<decltype(rhs)>;
                if constexpr (std::is_same_v<LHS, RHS>)
                {
                    return &lhs.get() == &rhs.get();
                }
                else
                {
                    return false;
                }
            },
            m_choice,
            other.m_choice);
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t Choice<D>::hash() const
{
    return std::visit([](const auto& arg) -> size_t { return loki::hash_combine(&arg.get()); }, m_choice);
}

template<dl::IsConceptOrRole D>
bool Choice<D>::test_match(const dl::Constructor<D>& constructor) const
{
    return std::visit([&constructor](const auto& arg) -> bool { return arg.get().test_match(constructor); }, m_choice);
}

template<dl::IsConceptOrRole D>
size_t Choice<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const ConstructorOrNonTerminalChoice<D>& Choice<D>::get_choice() const
{
    return m_choice;
}

template class Choice<Concept>;
template class Choice<Role>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
DerivationRule<D>::DerivationRule(size_t id, ChoiceList<D> choices) : m_id(id), m_choices(std::move(choices))
{
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::operator==(const DerivationRule& other) const
{
    if (this != &other)
    {
        if (m_choices.size() != other.m_choices.size())
        {
            return false;
        }
        for (size_t i = 0; i < m_choices.size(); ++i)
        {
            if (m_choices[i].get() != other.m_choices[i].get())
            {
                return false;
            }
        }
    }
    return true;
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::hash() const
{
    size_t seed = m_choices.size();
    std::for_each(m_choices.begin(), m_choices.end(), [&seed](const auto& choice) { loki::hash_combine(seed, &choice.get()); });
    return seed;
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::test_match(const dl::Constructor<D>& constructor) const
{
    return std::any_of(m_choices.begin(), m_choices.end(), [&constructor](const auto& choice) { return choice.get().test_match(constructor); });
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::get_id() const
{
    return m_id;
}

template<dl::IsConceptOrRole D>
const ChoiceList<D>& DerivationRule<D>::get_choices() const
{
    return m_choices;
}

template class DerivationRule<Concept>;
template class DerivationRule<Role>;

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
bool ConceptPredicateState<P>::is_equal(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptPredicateState<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::test_match(const dl::Constructor<Concept>& constructor) const
{
    return constructor.accept(grammar::ConceptPredicateStateVisitor<P>(*this));
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateState<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptPredicateState<Static>;
template class ConceptPredicateState<Fluent>;
template class ConceptPredicateState<Derived>;

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
bool ConceptPredicateGoal<P>::is_equal(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptPredicateGoal<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::test_match(const dl::Constructor<Concept>& constructor) const
{
    return constructor.accept(ConceptPredicateGoalVisitor<P>(*this));
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> ConceptPredicateGoal<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptPredicateGoal<Static>;
template class ConceptPredicateGoal<Fluent>;
template class ConceptPredicateGoal<Derived>;

/**
 * ConceptAnd
 */
ConceptAnd::ConceptAnd(size_t id, const Choice<Concept>& concept_left, const Choice<Concept>& concept_right) :
    m_id(id),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

bool ConceptAnd::operator==(const ConceptAnd& other) const
{
    if (this != &other)
    {
        return (&m_concept_left == &other.m_concept_left) && (&m_concept_right == &other.m_concept_right);
    }
    return true;
}

bool ConceptAnd::is_equal(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptAnd&>(other);
    return (*this == otherDerived);
}

size_t ConceptAnd::hash() const { return loki::hash_combine(&m_concept_left, &m_concept_right); }

bool ConceptAnd::test_match(const dl::Constructor<Concept>& constructor) const { return constructor.accept(ConceptAndVisitor(*this)); }

size_t ConceptAnd::get_id() const { return m_id; }

const Choice<Concept>& ConceptAnd::get_concept_left() const { return m_concept_left.get(); }

const Choice<Concept>& ConceptAnd::get_concept_right() const { return m_concept_right.get(); }

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
bool RolePredicateState<P>::is_equal(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RolePredicateState<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t RolePredicateState<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool RolePredicateState<P>::test_match(const dl::Constructor<Role>& constructor) const
{
    return constructor.accept(RolePredicateStateVisitor<P>(*this));
}

template<PredicateCategory P>
size_t RolePredicateState<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> RolePredicateState<P>::get_predicate() const
{
    return m_predicate;
}

template class RolePredicateState<Static>;
template class RolePredicateState<Fluent>;
template class RolePredicateState<Derived>;

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
bool RolePredicateGoal<P>::is_equal(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RolePredicateGoal<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::hash() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::test_match(const dl::Constructor<Role>& constructor) const
{
    return constructor.accept(RolePredicateGoalVisitor<P>(*this));
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::get_id() const
{
    return m_id;
}

template<PredicateCategory P>
Predicate<P> RolePredicateGoal<P>::get_predicate() const
{
    return m_predicate;
}

template class RolePredicateGoal<Static>;
template class RolePredicateGoal<Fluent>;
template class RolePredicateGoal<Derived>;

/**
 * RoleAnd
 */
RoleAnd::RoleAnd(size_t id, const Choice<Role>& role_left, const Choice<Role>& role_right) : m_id(id), m_role_left(role_left), m_role_right(role_right) {}

bool RoleAnd::operator==(const RoleAnd& other) const
{
    if (this != &other)
    {
        return (&m_role_left == &other.m_role_left) && (&m_role_right == &other.m_role_right);
    }
    return true;
}

bool RoleAnd::is_equal(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RoleAnd&>(other);
    return (*this == otherDerived);
}

size_t RoleAnd::hash() const { return loki::hash_combine(&m_role_left, &m_role_right); }

bool RoleAnd::test_match(const dl::Constructor<Role>& constructor) const { return constructor.accept(RoleAndVisitor(*this)); }

size_t RoleAnd::get_id() const { return m_id; }

const Choice<Role>& RoleAnd::get_role_left() const { return m_role_left.get(); }

const Choice<Role>& RoleAnd::get_role_right() const { return m_role_right.get(); }

}
