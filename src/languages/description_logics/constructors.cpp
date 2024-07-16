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

#include <mimir/languages/description_logics/constructors.hpp>

namespace mimir::dl
{

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
bool ConceptPredicateState<P>::is_equal_impl(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptPredicateState<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::hash_impl() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
void ConceptPredicateState<P>::evaluate_impl(EvaluationContext& context) const
{
    auto& bitset = context.get_denotation_builder<Concept>().get_bitset();
    for (const auto& atom : context.get_state_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_objects().at(0)->get_identifier());
        }
    }
}

template<>
void ConceptPredicateState<Static>::evaluate_impl(EvaluationContext& context) const
{
    auto& bitset = context.get_denotation_builder<Concept>().get_bitset();
    for (const auto& atom : context.get_state_atoms<Static>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_identifier());
        }
    }
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::accept_impl(const grammar::Visitor<Concept>& visitor) const
{
    return visitor.visit(*this);
}

template<PredicateCategory P>
size_t ConceptPredicateState<P>::get_id_impl() const
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
bool ConceptPredicateGoal<P>::is_equal_impl(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptPredicateGoal<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::hash_impl() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
void ConceptPredicateGoal<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_bitset();
    bitset.unset_all();

    // Compute result
    for (const auto& atom : context.get_goal_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_objects().at(0)->get_identifier());
        }
    }
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::accept_impl(const grammar::Visitor<Concept>& visitor) const
{
    return visitor.visit(*this);
}

template<PredicateCategory P>
size_t ConceptPredicateGoal<P>::get_id_impl() const
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

ConceptAnd::ConceptAnd(size_t id, const Constructor<Concept>& concept_left, const Constructor<Concept>& concept_right) :
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

bool ConceptAnd::is_equal_impl(const Constructor<Concept>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const ConceptAnd&>(other);
    return (*this == otherDerived);
}

size_t ConceptAnd::hash_impl() const { return loki::hash_combine(&m_concept_left, &m_concept_right); }

void ConceptAnd::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_concept_left.get().evaluate(context);
    const auto eval_right = m_concept_left.get().evaluate(context);

    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_bitset();
    bitset.unset_all();

    // Compute result
    bitset |= eval_left.get_bitset();
    bitset &= eval_right.get_bitset();
}

bool ConceptAnd::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(*this); }

size_t ConceptAnd::get_id_impl() const { return m_id; }

const Constructor<Concept>& ConceptAnd::get_concept_left() const { return m_concept_left.get(); }

const Constructor<Concept>& ConceptAnd::get_concept_right() const { return m_concept_right.get(); }

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
bool RolePredicateState<P>::is_equal_impl(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RolePredicateState<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t RolePredicateState<P>::hash_impl() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
void RolePredicateState<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_bitsets();

    // Compute result
    for (const auto& atom : context.get_state_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_identifier();
            const auto object_right_id = atom->get_objects().at(1)->get_identifier();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }
}

template<>
void RolePredicateState<Static>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_bitsets();

    // Compute result
    for (const auto& atom : context.get_state_atoms<Static>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_identifier();
            const auto object_right_id = atom->get_objects().at(1)->get_identifier();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }
}

template<PredicateCategory P>
bool RolePredicateState<P>::accept_impl(const grammar::Visitor<Role>& visitor) const
{
    return visitor.visit(*this);
}

template<PredicateCategory P>
size_t RolePredicateState<P>::get_id_impl() const
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
bool RolePredicateGoal<P>::is_equal_impl(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RolePredicateGoal<P>&>(other);
    return (*this == otherDerived);
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::hash_impl() const
{
    return loki::hash_combine(m_predicate);
}

template<PredicateCategory P>
void RolePredicateGoal<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_bitsets();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& atom : context.get_goal_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_identifier();
            const auto object_right_id = atom->get_objects().at(1)->get_identifier();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::accept_impl(const grammar::Visitor<Role>& visitor) const
{
    return visitor.visit(*this);
}

template<PredicateCategory P>
size_t RolePredicateGoal<P>::get_id_impl() const
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

RoleAnd::RoleAnd(size_t id, const Constructor<Role>& role_left, const Constructor<Role>& role_right) :
    m_id(id),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

bool RoleAnd::operator==(const RoleAnd& other) const
{
    if (this != &other)
    {
        return (&m_role_left == &other.m_role_left) && (&m_role_right == &other.m_role_right);
    }
    return true;
}

bool RoleAnd::is_equal_impl(const Constructor<Role>& other) const
{
    if (!this->type_equal(other))
    {
        return false;
    }
    const auto& otherDerived = static_cast<const RoleAnd&>(other);
    return (*this == otherDerived);
}

size_t RoleAnd::hash_impl() const { return loki::hash_combine(&m_role_left, &m_role_right); }

void RoleAnd::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_role_left.get().evaluate(context);
    const auto eval_right = m_role_left.get().evaluate(context);

    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_bitsets();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (size_t i = 0; i < eval_left.get_bitsets().size(); ++i)
    {
        auto& bitset = bitsets.at(i);
        bitset |= eval_left.get_bitsets().at(i);
        bitset &= eval_right.get_bitsets().at(i);
    }
}

bool RoleAnd::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(*this); }

size_t RoleAnd::get_id_impl() const { return m_id; }

const Constructor<Role>& RoleAnd::get_role_left() const { return m_role_left.get(); }

const Constructor<Role>& RoleAnd::get_role_right() const { return m_role_right.get(); }
}
