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

#include "mimir/languages/description_logics/constructors.hpp"

#include "mimir/common/hash.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_function.hpp"
#include "mimir/formalism/ground_function_expressions.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/grammar_visitors_interface.hpp"

namespace mimir::dl
{

/**
 * ConceptBot
 */

ConceptBotImpl::ConceptBotImpl(Index index) : m_index(index) {}

void ConceptBotImpl::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Result is computed.
}

bool ConceptBotImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptBotImpl::get_index() const { return m_index; }

/**
 * ConceptTop
 */

ConceptTopImpl::ConceptTopImpl(Index index) : m_index(index) {}

void ConceptTopImpl::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    for (size_t i = 0; i < context.get_num_objects(); ++i)
    {
        bitset.set(i);
    }
}

bool ConceptTopImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptTopImpl::get_index() const { return m_index; }

/**
 * ConceptAtomicState
 */

template<PredicateCategory P>
ConceptAtomicStateImpl<P>::ConceptAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
void ConceptAtomicStateImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    for (const auto& atom : context.get_state_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_objects().at(0)->get_index());
        }
    }
}

template<>
void ConceptAtomicStateImpl<Static>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    for (const auto& atom : context.get_state_atoms<Static>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_index());
        }
    }
}

template<PredicateCategory P>
bool ConceptAtomicStateImpl<P>::accept_impl(const grammar::Visitor<Concept>& visitor) const
{
    return visitor.visit(this);
}

template<PredicateCategory P>
Index ConceptAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> ConceptAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptAtomicStateImpl<Static>;
template class ConceptAtomicStateImpl<Fluent>;
template class ConceptAtomicStateImpl<Derived>;

/**
 * ConceptAtomicGoal
 */

template<PredicateCategory P>
ConceptAtomicGoalImpl<P>::ConceptAtomicGoalImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
void ConceptAtomicGoalImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    for (const auto& atom : context.get_goal_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_objects().at(0)->get_index());
        }
    }
}

template<PredicateCategory P>
bool ConceptAtomicGoalImpl<P>::accept_impl(const grammar::Visitor<Concept>& visitor) const
{
    return visitor.visit(this);
}

template<PredicateCategory P>
Index ConceptAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> ConceptAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptAtomicGoalImpl<Static>;
template class ConceptAtomicGoalImpl<Fluent>;
template class ConceptAtomicGoalImpl<Derived>;

/**
 * ConceptIntersection
 */

ConceptIntersectionImpl::ConceptIntersectionImpl(Index index, Constructor<Concept> concept_left, Constructor<Concept> concept_right) :
    m_index(index),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

void ConceptIntersectionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_concept_left->evaluate(context);
    const auto eval_right = m_concept_left->evaluate(context);

    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    bitset |= eval_left->get_data();
    bitset &= eval_right->get_data();
}

bool ConceptIntersectionImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptIntersectionImpl::get_index() const { return m_index; }

Constructor<Concept> ConceptIntersectionImpl::get_concept_left() const { return m_concept_left; }

Constructor<Concept> ConceptIntersectionImpl::get_concept_right() const { return m_concept_right; }

/**
 * ConceptUnion
 */

ConceptUnionImpl::ConceptUnionImpl(Index index, Constructor<Concept> concept_left, Constructor<Concept> concept_right) :
    m_index(index),
    m_concept_left(concept_left),
    m_concept_right(concept_right)
{
}

void ConceptUnionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_concept_left->evaluate(context);
    const auto eval_right = m_concept_left->evaluate(context);

    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    bitset |= eval_left->get_data();
    bitset |= eval_right->get_data();
}

bool ConceptUnionImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptUnionImpl::get_index() const { return m_index; }

Constructor<Concept> ConceptUnionImpl::get_concept_left() const { return m_concept_left; }

Constructor<Concept> ConceptUnionImpl::get_concept_right() const { return m_concept_right; }

/**
 * ConceptNegation
 */

ConceptNegationImpl::ConceptNegationImpl(Index index, Constructor<Concept> concept_) : m_index(index), m_concept(concept_) {}

void ConceptNegationImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval = m_concept->evaluate(context);

    // Fetch data
    auto& bitset = context.get_denotation_builder<Concept>().get_data();
    bitset.unset_all();

    // Compute result
    for (size_t i = 0; i < context.get_num_objects(); ++i)
    {
        if (!eval->get_data().get(i))
        {
            bitset.set(i);
        }
    }
}

bool ConceptNegationImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptNegationImpl::get_index() const { return m_index; }

Constructor<Concept> ConceptNegationImpl::get_concept() const { return m_concept; }

/**
 * ConceptValueRestriction
 */

ConceptValueRestrictionImpl::ConceptValueRestrictionImpl(Index index, Constructor<Role> role_, Constructor<Concept> concept_) :
    m_index(index),
    m_role(role_),
    m_concept(concept_)
{
}

void ConceptValueRestrictionImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool ConceptValueRestrictionImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptValueRestrictionImpl::get_index() const { return m_index; }

Constructor<Role> ConceptValueRestrictionImpl::get_role() const { return m_role; }

Constructor<Concept> ConceptValueRestrictionImpl::get_concept() const { return m_concept; }

/**
 * ConceptExistentialQuantification
 */

ConceptExistentialQuantificationImpl::ConceptExistentialQuantificationImpl(Index index, Constructor<Role> role_, Constructor<Concept> concept_) :
    m_index(index),
    m_role(role_),
    m_concept(concept_)
{
}

void ConceptExistentialQuantificationImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool ConceptExistentialQuantificationImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptExistentialQuantificationImpl::get_index() const { return m_index; }

Constructor<Role> ConceptExistentialQuantificationImpl::get_role() const { return m_role; }

Constructor<Concept> ConceptExistentialQuantificationImpl::get_concept() const { return m_concept; }

/**
 * ConceptRoleValueMapContainment
 */

ConceptRoleValueMapContainmentImpl::ConceptRoleValueMapContainmentImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right) :
    m_index(index),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

void ConceptRoleValueMapContainmentImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool ConceptRoleValueMapContainmentImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptRoleValueMapContainmentImpl::get_index() const { return m_index; }

Constructor<Role> ConceptRoleValueMapContainmentImpl::get_role_left() const { return m_role_left; }

Constructor<Role> ConceptRoleValueMapContainmentImpl::get_role_right() const { return m_role_right; }

/**
 * ConceptRoleValueMapEquality
 */

ConceptRoleValueMapEqualityImpl::ConceptRoleValueMapEqualityImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right) :
    m_index(index),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

void ConceptRoleValueMapEqualityImpl::evaluate_impl(EvaluationContext& context) const {}

bool ConceptRoleValueMapEqualityImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptRoleValueMapEqualityImpl::get_index() const { return m_index; }

Constructor<Role> ConceptRoleValueMapEqualityImpl::get_role_left() const { return m_role_left; }

Constructor<Role> ConceptRoleValueMapEqualityImpl::get_role_right() const { return m_role_right; }

/**
 * ConceptNominalImpl
 */

ConceptNominalImpl::ConceptNominalImpl(Index index, Object object) : m_index(index), m_object(object) {}

void ConceptNominalImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool ConceptNominalImpl::accept_impl(const grammar::Visitor<Concept>& visitor) const { return visitor.visit(this); }

Index ConceptNominalImpl::get_index() const { return m_index; }

Object ConceptNominalImpl::get_object() const { return m_object; }

/**
 * RoleUniversal
 */

RoleUniversalImpl::RoleUniversalImpl(Index index) : m_index(index) {}

void RoleUniversalImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleUniversalImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleUniversalImpl::get_index() const { return m_index; }

/**
 * RoleAtomicState
 */

template<PredicateCategory P>
RoleAtomicStateImpl<P>::RoleAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
void RoleAtomicStateImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_data();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& atom : context.get_state_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_index();
            const auto object_right_id = atom->get_objects().at(1)->get_index();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }
}

template<>
void RoleAtomicStateImpl<Static>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_data();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& atom : context.get_state_atoms<Static>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_index();
            const auto object_right_id = atom->get_objects().at(1)->get_index();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }
}

template<PredicateCategory P>
bool RoleAtomicStateImpl<P>::accept_impl(const grammar::Visitor<Role>& visitor) const
{
    return visitor.visit(this);
}

template<PredicateCategory P>
Index RoleAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> RoleAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RoleAtomicStateImpl<Static>;
template class RoleAtomicStateImpl<Fluent>;
template class RoleAtomicStateImpl<Derived>;

/**
 * RoleAtomicGoal
 */

template<PredicateCategory P>
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<PredicateCategory P>
void RoleAtomicGoalImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_data();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& atom : context.get_goal_atoms<P>())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_index();
            const auto object_right_id = atom->get_objects().at(1)->get_index();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }
}

template<PredicateCategory P>
bool RoleAtomicGoalImpl<P>::accept_impl(const grammar::Visitor<Role>& visitor) const
{
    return visitor.visit(this);
}

template<PredicateCategory P>
Index RoleAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<PredicateCategory P>
Predicate<P> RoleAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RoleAtomicGoalImpl<Static>;
template class RoleAtomicGoalImpl<Fluent>;
template class RoleAtomicGoalImpl<Derived>;

/**
 * RoleIntersection
 */

RoleIntersectionImpl::RoleIntersectionImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right) :
    m_index(index),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

void RoleIntersectionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_role_left->evaluate(context);
    const auto eval_right = m_role_left->evaluate(context);

    // Fetch data
    auto& bitsets = context.get_denotation_builder<Role>().get_data();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (size_t i = 0; i < eval_left->get_data().size(); ++i)
    {
        auto& bitset = bitsets.at(i);
        bitset |= eval_left->get_data().at(i);
        bitset &= eval_right->get_data().at(i);
    }
}

bool RoleIntersectionImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleIntersectionImpl::get_index() const { return m_index; }

Constructor<Role> RoleIntersectionImpl::get_role_left() const { return m_role_left; }

Constructor<Role> RoleIntersectionImpl::get_role_right() const { return m_role_right; }

/**
 * RoleUnion
 */

RoleUnionImpl::RoleUnionImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right) :
    m_index(index),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

void RoleUnionImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleUnionImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleUnionImpl::get_index() const { return m_index; }

Constructor<Role> RoleUnionImpl::get_role_left() const { return m_role_left; }

Constructor<Role> RoleUnionImpl::get_role_right() const { return m_role_right; }

/**
 * RoleComplement
 */

RoleComplementImpl::RoleComplementImpl(Index index, Constructor<Role> role_) : m_index(index), m_role(role_) {}

void RoleComplementImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleComplementImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleComplementImpl::get_index() const { return m_index; }

Constructor<Role> RoleComplementImpl::get_role() const { return m_role; }

/**
 * RoleInverse
 */

RoleInverseImpl::RoleInverseImpl(Index index, Constructor<Role> role_) : m_index(index), m_role(role_) {}

void RoleInverseImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleInverseImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleInverseImpl::get_index() const { return m_index; }

Constructor<Role> RoleInverseImpl::get_role() const { return m_role; }

/**
 * RoleComposition
 */

RoleCompositionImpl::RoleCompositionImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right) :
    m_index(index),
    m_role_left(role_left),
    m_role_right(role_right)
{
}

void RoleCompositionImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleCompositionImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleCompositionImpl::get_index() const { return m_index; }

Constructor<Role> RoleCompositionImpl::get_role_left() const { return m_role_left; }

Constructor<Role> RoleCompositionImpl::get_role_right() const { return m_role_right; }

/**
 * RoleTransitiveClosure
 */

RoleTransitiveClosureImpl::RoleTransitiveClosureImpl(Index index, Constructor<Role> role_) : m_index(index), m_role(role_) {}

void RoleTransitiveClosureImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleTransitiveClosureImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleTransitiveClosureImpl::get_index() const { return m_index; }

Constructor<Role> RoleTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleReflexiveTransitiveClosure
 */

RoleReflexiveTransitiveClosureImpl::RoleReflexiveTransitiveClosureImpl(Index index, Constructor<Role> role_) : m_index(index), m_role(role_) {}

void RoleReflexiveTransitiveClosureImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleReflexiveTransitiveClosureImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleReflexiveTransitiveClosureImpl::get_index() const { return m_index; }

Constructor<Role> RoleReflexiveTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleRestriction
 */

RoleRestrictionImpl::RoleRestrictionImpl(Index index, Constructor<Role> role_, Constructor<Concept> concept_) :
    m_index(index),
    m_role(role_),
    m_concept(concept_)
{
}

void RoleRestrictionImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleRestrictionImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleRestrictionImpl::get_index() const { return m_index; }

Constructor<Role> RoleRestrictionImpl::get_role() const { return m_role; }

Constructor<Concept> RoleRestrictionImpl::get_concept() const { return m_concept; }

/**
 * RoleIdentity
 */

RoleIdentityImpl::RoleIdentityImpl(Index index, Constructor<Concept> concept_) : m_index(index), m_concept(concept_) {}

void RoleIdentityImpl::evaluate_impl(EvaluationContext& context) const { throw std::runtime_error("Not implemented"); }

bool RoleIdentityImpl::accept_impl(const grammar::Visitor<Role>& visitor) const { return visitor.visit(this); }

Index RoleIdentityImpl::get_index() const { return m_index; }

Constructor<Concept> RoleIdentityImpl::get_concept() const { return m_concept; }

}
