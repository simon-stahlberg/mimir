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

Denotation<Concept> ConceptAnd::evaluate(EvaluationContext& context) const
{
    // Try to access cached result
    auto denotation = context.concept_denotation_repository.get_if(this, context.state);
    if (denotation.has_value())
    {
        return denotation.value();
    }

    // Fetch data
    auto& bitset = context.concept_denotation.get_bitset();
    context.concept_denotation.get_bitset().unset_all();

    // Evaluate children
    const auto eval_left = m_concept_left.evaluate(context);
    const auto eval_right = m_concept_left.evaluate(context);

    // Compute result
    bitset |= eval_left.get_bitset();
    bitset &= eval_right.get_bitset();

    // Store and return result;
    context.concept_denotation.get_flatmemory_builder().finish();
    return context.concept_denotation_repository.insert(this, context.state, context.concept_denotation);
}

bool ConceptAnd::accept(const ConceptVisitor& visitor) const { return visitor.visit(*this); }

size_t ConceptAnd::get_id() const { return m_id; }

const Constructor<Concept>& ConceptAnd::get_concept_left() const { return m_concept_left; }

const Constructor<Concept>& ConceptAnd::get_concept_right() const { return m_concept_right; }

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

Denotation<Role> RoleAnd::evaluate(EvaluationContext& context) const
{
    // Try to access cached result
    auto denotation = context.role_denotation_repository.get_if(this, context.state);
    if (denotation.has_value())
    {
        return denotation.value();
    }

    // Fetch data
    for (auto& bitset : context.role_denotation.get_bitsets())
    {
        bitset.unset_all();
    }

    // Evaluate children
    const auto eval_left = m_role_left.evaluate(context);
    const auto eval_right = m_role_left.evaluate(context);

    // Compute result
    for (size_t i = 0; i < eval_left.get_bitsets().size(); ++i)
    {
        auto& bitset = context.role_denotation.get_bitsets()[i];
        bitset |= eval_left.get_bitsets()[i];
        bitset &= eval_right.get_bitsets()[i];
    }

    // Store and return result;
    context.role_denotation.get_flatmemory_builder().finish();
    return context.role_denotation_repository.insert(this, context.role_denotation);
}

bool RoleAnd::accept(const RoleVisitor& visitor) const { return visitor.visit(*this); }

size_t RoleAnd::get_id() const { return m_id; }

const Constructor<Role>& RoleAnd::get_role_left() const { return m_role_left; }

const Constructor<Role>& RoleAnd::get_role_right() const { return m_role_right; }
}
