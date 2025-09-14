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
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"
#include "mimir/languages/description_logics/constructor_visitor_formatter.hpp"
#include "mimir/languages/description_logics/constructor_visitor_interface.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"

#include <queue>

using namespace mimir::formalism;

namespace mimir::languages::dl
{

/**
 * ConceptBot
 */

ConceptBotImpl::ConceptBotImpl(Index index) : m_index(index) {}

void ConceptBotImpl::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
}

void ConceptBotImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptBotImpl::get_complexity_impl() const { return 1; }

Index ConceptBotImpl::get_index() const { return m_index; }

/**
 * ConceptTop
 */

ConceptTopImpl::ConceptTopImpl(Index index) : m_index(index) {}

void ConceptTopImpl::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    for (size_t i = 0; i < num_objects; ++i)
    {
        bitset.set(i);
    }
}

void ConceptTopImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptTopImpl::get_complexity_impl() const { return 1; }

Index ConceptTopImpl::get_index() const { return m_index; }

/**
 * ConceptAtomicState
 */

template<IsStaticOrFluentOrDerivedTag P>
ConceptAtomicStateImpl<P>::ConceptAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicStateImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    for (const auto& atom_index : context.get_state().get_atoms<P>())
    {
        const auto atom = context.get_problem()->get_repositories().template get_ground_atom<P>(atom_index);

        if (atom->get_predicate() == m_predicate)
        {
            // Ensure that object index is within bounds.
            assert(atom->get_objects().at(0)->get_index() < context.get_problem()->get_problem_and_domain_objects().size());

            bitset.set(atom->get_objects().at(0)->get_index());
        }
    }
}

template<>
void ConceptAtomicStateImpl<StaticTag>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    for (const auto& atom : context.get_problem()->get_static_initial_atoms())
    {
        if (atom->get_predicate() == m_predicate)
        {
            // Ensure that object index is within bounds.
            assert(atom->get_objects().at(0)->get_index() < context.get_problem()->get_problem_and_domain_objects().size());

            bitset.set(atom->get_objects().at(0)->get_index());
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicStateImpl<P>::accept_impl(IVisitor& visitor) const
{
    return visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
size_t ConceptAtomicStateImpl<P>::get_complexity_impl() const
{
    return 1;
}

template<IsStaticOrFluentOrDerivedTag P>
Index ConceptAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> ConceptAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class ConceptAtomicStateImpl<StaticTag>;
template class ConceptAtomicStateImpl<FluentTag>;
template class ConceptAtomicStateImpl<DerivedTag>;

/**
 * ConceptAtomicGoal
 */

template<IsStaticOrFluentOrDerivedTag P>
ConceptAtomicGoalImpl<P>::ConceptAtomicGoalImpl(Index index, Predicate<P> predicate, bool polarity) :
    m_index(index),
    m_predicate(predicate),
    m_polarity(polarity)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicGoalImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    for (const auto& literal : context.get_problem()->get_goal_condition<P>())
    {
        const auto atom = literal->get_atom();

        if (literal->get_polarity() == get_polarity() && atom->get_predicate() == m_predicate)
        {
            // Ensure that object index is within bounds.
            assert(atom->get_objects().at(0)->get_index() < context.get_problem()->get_problem_and_domain_objects().size());

            bitset.set(atom->get_objects().at(0)->get_index());
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void ConceptAtomicGoalImpl<P>::accept_impl(IVisitor& visitor) const
{
    return visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
size_t ConceptAtomicGoalImpl<P>::get_complexity_impl() const
{
    return 1;
}

template<IsStaticOrFluentOrDerivedTag P>
Index ConceptAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> ConceptAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<IsStaticOrFluentOrDerivedTag P>
bool ConceptAtomicGoalImpl<P>::get_polarity() const
{
    return m_polarity;
}

template class ConceptAtomicGoalImpl<StaticTag>;
template class ConceptAtomicGoalImpl<FluentTag>;
template class ConceptAtomicGoalImpl<DerivedTag>;

/**
 * ConceptIntersection
 */

ConceptIntersectionImpl::ConceptIntersectionImpl(Index index, Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_right_concept(right_concept)
{
}

void ConceptIntersectionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_left_concept->evaluate(context);
    const auto eval_right = m_right_concept->evaluate(context);

    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset = eval_left->get_data();
    bitset &= eval_right->get_data();
}

void ConceptIntersectionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptIntersectionImpl::get_complexity_impl() const { return m_left_concept->get_complexity() + m_right_concept->get_complexity() + 1; }

Index ConceptIntersectionImpl::get_index() const { return m_index; }

Constructor<ConceptTag> ConceptIntersectionImpl::get_left_concept() const { return m_left_concept; }

Constructor<ConceptTag> ConceptIntersectionImpl::get_right_concept() const { return m_right_concept; }

/**
 * ConceptUnion
 */

ConceptUnionImpl::ConceptUnionImpl(Index index, Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_right_concept(right_concept)
{
}

void ConceptUnionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_left_concept->evaluate(context);
    const auto eval_right = m_right_concept->evaluate(context);

    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset = eval_left->get_data();
    bitset |= eval_right->get_data();
}

void ConceptUnionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptUnionImpl::get_complexity_impl() const { return m_left_concept->get_complexity() + m_right_concept->get_complexity() + 1; }

Index ConceptUnionImpl::get_index() const { return m_index; }

Constructor<ConceptTag> ConceptUnionImpl::get_left_concept() const { return m_left_concept; }

Constructor<ConceptTag> ConceptUnionImpl::get_right_concept() const { return m_right_concept; }

/**
 * ConceptNegation
 */

ConceptNegationImpl::ConceptNegationImpl(Index index, Constructor<ConceptTag> concept_) : m_index(index), m_concept(concept_) {}

void ConceptNegationImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval = m_concept->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset = eval->get_data();
    bitset.negate(num_objects);
}

void ConceptNegationImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptNegationImpl::get_complexity_impl() const { return m_concept->get_complexity() + 1; }

Index ConceptNegationImpl::get_index() const { return m_index; }

Constructor<ConceptTag> ConceptNegationImpl::get_concept() const { return m_concept; }

/**
 * ConceptValueRestriction
 */

ConceptValueRestrictionImpl::ConceptValueRestrictionImpl(Index index, Constructor<RoleTag> role_, Constructor<ConceptTag> concept_) :
    m_index(index),
    m_role(role_),
    m_concept(concept_)
{
}

void ConceptValueRestrictionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_role = m_role->evaluate(context);
    const auto eval_concept = m_concept->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();
    auto& scratch_bitset = boost::hana::at_key(context.get_scratch_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.set_all(num_objects, true);
    for (size_t i = 0; i < num_objects; ++i)
    {
        scratch_bitset = eval_concept->get_data();
        scratch_bitset.negate(num_objects);
        scratch_bitset &= eval_role->get_data().at(i);
        if (scratch_bitset.any())
            bitset.unset(i);
    }
}

void ConceptValueRestrictionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptValueRestrictionImpl::get_complexity_impl() const { return m_role->get_complexity() + m_concept->get_complexity() + 1; }

Index ConceptValueRestrictionImpl::get_index() const { return m_index; }

Constructor<RoleTag> ConceptValueRestrictionImpl::get_role() const { return m_role; }

Constructor<ConceptTag> ConceptValueRestrictionImpl::get_concept() const { return m_concept; }

/**
 * ConceptExistentialQuantification
 */

ConceptExistentialQuantificationImpl::ConceptExistentialQuantificationImpl(Index index, Constructor<RoleTag> role_, Constructor<ConceptTag> concept_) :
    m_index(index),
    m_role(role_),
    m_concept(concept_)
{
}

void ConceptExistentialQuantificationImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_role = m_role->evaluate(context);
    const auto eval_concept = m_concept->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();
    auto& scratch_bitset = boost::hana::at_key(context.get_scratch_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    for (size_t i = 0; i < num_objects; ++i)
    {
        scratch_bitset = eval_concept->get_data();
        scratch_bitset &= eval_role->get_data().at(i);
        if (scratch_bitset.any())
            bitset.set(i);
    }
}

void ConceptExistentialQuantificationImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptExistentialQuantificationImpl::get_complexity_impl() const { return m_role->get_complexity() + m_concept->get_complexity() + 1; }

Index ConceptExistentialQuantificationImpl::get_index() const { return m_index; }

Constructor<RoleTag> ConceptExistentialQuantificationImpl::get_role() const { return m_role; }

Constructor<ConceptTag> ConceptExistentialQuantificationImpl::get_concept() const { return m_concept; }

/**
 * ConceptRoleValueMapContainment
 */

ConceptRoleValueMapContainmentImpl::ConceptRoleValueMapContainmentImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

void ConceptRoleValueMapContainmentImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left_role = m_left_role->evaluate(context);
    const auto eval_right_role = m_right_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();
    auto& scratch_bitset = boost::hana::at_key(context.get_scratch_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    bitset.set_all(num_objects, true);
    for (size_t i = 0; i < num_objects; ++i)
    {
        scratch_bitset = eval_right_role->get_data().at(i);
        scratch_bitset.negate(num_objects);
        scratch_bitset &= eval_left_role->get_data().at(i);
        if (scratch_bitset.any())
            bitset.unset(i);
    }
}

void ConceptRoleValueMapContainmentImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptRoleValueMapContainmentImpl::get_complexity_impl() const { return m_left_role->get_complexity() + m_right_role->get_complexity() + 1; }

Index ConceptRoleValueMapContainmentImpl::get_index() const { return m_index; }

Constructor<RoleTag> ConceptRoleValueMapContainmentImpl::get_left_role() const { return m_left_role; }

Constructor<RoleTag> ConceptRoleValueMapContainmentImpl::get_right_role() const { return m_right_role; }

/**
 * ConceptRoleValueMapEquality
 */

ConceptRoleValueMapEqualityImpl::ConceptRoleValueMapEqualityImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

void ConceptRoleValueMapEqualityImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left_role = m_left_role->evaluate(context);
    const auto eval_right_role = m_right_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();
    auto& scratch_bitset = boost::hana::at_key(context.get_scratch_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.set_all(num_objects, true);
    for (size_t i = 0; i < num_objects; ++i)
    {
        scratch_bitset = eval_right_role->get_data().at(i);
        scratch_bitset ^= eval_left_role->get_data().at(i);
        if (scratch_bitset.any())
            bitset.unset(i);
    }
}

void ConceptRoleValueMapEqualityImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptRoleValueMapEqualityImpl::get_complexity_impl() const { return m_left_role->get_complexity() + m_right_role->get_complexity() + 1; }

Index ConceptRoleValueMapEqualityImpl::get_index() const { return m_index; }

Constructor<RoleTag> ConceptRoleValueMapEqualityImpl::get_left_role() const { return m_left_role; }

Constructor<RoleTag> ConceptRoleValueMapEqualityImpl::get_right_role() const { return m_right_role; }

/**
 * ConceptNominalImpl
 */

ConceptNominalImpl::ConceptNominalImpl(Index index, Object object) : m_index(index), m_object(object) {}

void ConceptNominalImpl::evaluate_impl(EvaluationContext& context) const
{
    // Ensure that object index is within bounds.
    assert(m_object->get_index() < context.get_problem()->get_problem_and_domain_objects().size());

    // Fetch data
    auto& bitset = boost::hana::at_key(context.get_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitset.unset_all();
    bitset.set(m_object->get_index());
}

void ConceptNominalImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t ConceptNominalImpl::get_complexity_impl() const { return 1; }

Index ConceptNominalImpl::get_index() const { return m_index; }

Object ConceptNominalImpl::get_object() const { return m_object; }

/**
 * RoleUniversal
 */

RoleUniversalImpl::RoleUniversalImpl(Index index) : m_index(index) {}

void RoleUniversalImpl::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.set_all(num_objects, true);
    }
}

void RoleUniversalImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleUniversalImpl::get_complexity_impl() const { return 1; }

Index RoleUniversalImpl::get_index() const { return m_index; }

/**
 * RoleAtomicState
 */

template<IsStaticOrFluentOrDerivedTag P>
RoleAtomicStateImpl<P>::RoleAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicStateImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }
    for (const auto& atom_index : context.get_state().get_atoms<P>())
    {
        const auto atom = context.get_problem()->get_repositories().template get_ground_atom<P>(atom_index);

        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_index = atom->get_objects().at(0)->get_index();
            const auto object_right_index = atom->get_objects().at(1)->get_index();

            // Ensure that object index is within bounds.
            assert(object_left_index < num_objects);
            assert(object_right_index < num_objects);

            bitsets.at(object_left_index).set(object_right_index);
        }
    }
}

template<>
void RoleAtomicStateImpl<StaticTag>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }
    for (const auto& atom : context.get_problem()->get_static_initial_atoms())
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_index = atom->get_objects().at(0)->get_index();
            const auto object_right_index = atom->get_objects().at(1)->get_index();

            // Ensure that object index is within bounds.
            assert(object_left_index < num_objects);
            assert(object_right_index < num_objects);

            bitsets.at(object_left_index).set(object_right_index);
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicStateImpl<P>::accept_impl(IVisitor& visitor) const
{
    return visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
size_t RoleAtomicStateImpl<P>::get_complexity_impl() const
{
    return 1;
}

template<IsStaticOrFluentOrDerivedTag P>
Index RoleAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> RoleAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class RoleAtomicStateImpl<StaticTag>;
template class RoleAtomicStateImpl<FluentTag>;
template class RoleAtomicStateImpl<DerivedTag>;

/**
 * RoleAtomicGoal
 */

template<IsStaticOrFluentOrDerivedTag P>
RoleAtomicGoalImpl<P>::RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool polarity) : m_index(index), m_predicate(predicate), m_polarity(polarity)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicGoalImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }
    for (const auto& literal : context.get_problem()->get_goal_condition<P>())
    {
        const auto atom = literal->get_atom();

        if (literal->get_polarity() == get_polarity() && atom->get_predicate() == m_predicate)
        {
            const auto object_left_index = atom->get_objects().at(0)->get_index();
            const auto object_right_index = atom->get_objects().at(1)->get_index();

            // Ensure that object index is within bounds.
            assert(object_left_index < num_objects);
            assert(object_right_index < num_objects);

            bitsets.at(object_left_index).set(object_right_index);
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void RoleAtomicGoalImpl<P>::accept_impl(IVisitor& visitor) const
{
    return visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
size_t RoleAtomicGoalImpl<P>::get_complexity_impl() const
{
    return 1;
}

template<IsStaticOrFluentOrDerivedTag P>
Index RoleAtomicGoalImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> RoleAtomicGoalImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<IsStaticOrFluentOrDerivedTag P>
bool RoleAtomicGoalImpl<P>::get_polarity() const
{
    return m_polarity;
}

template class RoleAtomicGoalImpl<StaticTag>;
template class RoleAtomicGoalImpl<FluentTag>;
template class RoleAtomicGoalImpl<DerivedTag>;

/**
 * RoleIntersection
 */

RoleIntersectionImpl::RoleIntersectionImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

void RoleIntersectionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    const auto eval_left = m_left_role->evaluate(context);
    const auto eval_right = m_right_role->evaluate(context);

    // Fetch data
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (size_t i = 0; i < num_objects; ++i)
    {
        auto& bitset = bitsets.at(i);
        bitset = eval_left->get_data().at(i);
        bitset &= eval_right->get_data().at(i);
    }
}

void RoleIntersectionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleIntersectionImpl::get_complexity_impl() const { return m_left_role->get_complexity() + m_right_role->get_complexity() + 1; }

Index RoleIntersectionImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleIntersectionImpl::get_left_role() const { return m_left_role; }

Constructor<RoleTag> RoleIntersectionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleUnion
 */

RoleUnionImpl::RoleUnionImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

void RoleUnionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate children
    const auto eval_left = m_left_role->evaluate(context);
    const auto eval_right = m_right_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (size_t i = 0; i < num_objects; ++i)
    {
        auto& bitset = bitsets.at(i);
        bitset = eval_left->get_data().at(i);
        bitset |= eval_right->get_data().at(i);
    }
}

void RoleUnionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleUnionImpl::get_complexity_impl() const { return m_left_role->get_complexity() + m_right_role->get_complexity() + 1; }

Index RoleUnionImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleUnionImpl::get_left_role() const { return m_left_role; }

Constructor<RoleTag> RoleUnionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleComplement
 */

RoleComplementImpl::RoleComplementImpl(Index index, Constructor<RoleTag> role_) : m_index(index), m_role(role_) {}

void RoleComplementImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_role = m_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (size_t i = 0; i < num_objects; ++i)
    {
        auto& bitset = bitsets.at(i);
        bitset = eval_role->get_data().at(i);
        bitset.negate(num_objects);
    }
}

void RoleComplementImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleComplementImpl::get_complexity_impl() const { return m_role->get_complexity() + 1; }

Index RoleComplementImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleComplementImpl::get_role() const { return m_role; }

/**
 * RoleInverse
 */

RoleInverseImpl::RoleInverseImpl(Index index, Constructor<RoleTag> role_) : m_index(index), m_role(role_) {}

void RoleInverseImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_role = m_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }
    for (size_t i = 0; i < num_objects; ++i)
    {
        for (size_t j = 0; j < num_objects; ++j)
        {
            if (eval_role->get_data().at(i).get(j))
            {
                bitsets.at(j).set(i);
            }
        }
    }
}

void RoleInverseImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleInverseImpl::get_complexity_impl() const { return m_role->get_complexity() + 1; }

Index RoleInverseImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleInverseImpl::get_role() const { return m_role; }

/**
 * RoleComposition
 */

RoleCompositionImpl::RoleCompositionImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role) :
    m_index(index),
    m_left_role(left_role),
    m_right_role(right_role)
{
}

void RoleCompositionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_left_role = m_left_role->evaluate(context);
    const auto eval_right_role = m_right_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    for (size_t i = 0; i < num_objects; ++i)
    {
        for (const auto j : eval_left_role->get_data().at(i))
        {
            bitsets.at(i) |= eval_right_role->get_data().at(j);
        }
    }
}

void RoleCompositionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleCompositionImpl::get_complexity_impl() const { return m_left_role->get_complexity() + m_right_role->get_complexity() + 1; }

Index RoleCompositionImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleCompositionImpl::get_left_role() const { return m_left_role; }

Constructor<RoleTag> RoleCompositionImpl::get_right_role() const { return m_right_role; }

/**
 * RoleTransitiveClosure
 */

RoleTransitiveClosureImpl::RoleTransitiveClosureImpl(Index index, Constructor<RoleTag> role_) : m_index(index), m_role(role_) {}

void RoleTransitiveClosureImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_role = m_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (size_t i = 0; i < num_objects; ++i)
    {
        bitsets.at(i) = eval_role->get_data().at(i);
    }
    for (size_t k = 0; k < num_objects; ++k)
    {
        for (size_t i = 0; i < num_objects; ++i)
        {
            if (bitsets.at(i).get(k))
            {
                bitsets.at(i) |= bitsets.at(k);
            }
        }
    }
}

void RoleTransitiveClosureImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleTransitiveClosureImpl::get_complexity_impl() const { return m_role->get_complexity() + 1; }

Index RoleTransitiveClosureImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleReflexiveTransitiveClosure
 */

RoleReflexiveTransitiveClosureImpl::RoleReflexiveTransitiveClosureImpl(Index index, Constructor<RoleTag> role_) : m_index(index), m_role(role_) {}

void RoleReflexiveTransitiveClosureImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_role = m_role->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (size_t i = 0; i < num_objects; ++i)
    {
        bitsets.at(i) = eval_role->get_data().at(i);
    }
    for (size_t k = 0; k < num_objects; ++k)
    {
        for (size_t i = 0; i < num_objects; ++i)
        {
            if (bitsets.at(i).get(k))
            {
                bitsets.at(i) |= bitsets.at(k);
            }
        }
    }
    // Add reflexive part
    for (size_t i = 0; i < num_objects; ++i)
    {
        bitsets.at(i).set(i);
    }
}

void RoleReflexiveTransitiveClosureImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleReflexiveTransitiveClosureImpl::get_complexity_impl() const { return m_role->get_complexity() + 1; }

Index RoleReflexiveTransitiveClosureImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleReflexiveTransitiveClosureImpl::get_role() const { return m_role; }

/**
 * RoleRestriction
 */

RoleRestrictionImpl::RoleRestrictionImpl(Index index, Constructor<RoleTag> role_, Constructor<ConceptTag> concept_) :
    m_index(index),
    m_role(role_),
    m_concept(concept_)
{
}

void RoleRestrictionImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_role = m_role->evaluate(context);
    const auto eval_concept = m_concept->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();
    auto& scratch_bitset = boost::hana::at_key(context.get_scratch_builders(), boost::hana::type<ConceptTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }
    for (size_t i = 0; i < num_objects; ++i)
    {
        scratch_bitset = eval_concept->get_data();
        scratch_bitset &= eval_role->get_data().at(i);

        for (const auto& j : scratch_bitset)
        {
            bitsets.at(i).set(j);
        }
    }
}

void RoleRestrictionImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleRestrictionImpl::get_complexity_impl() const { return m_role->get_complexity() + m_concept->get_complexity() + 1; }

Index RoleRestrictionImpl::get_index() const { return m_index; }

Constructor<RoleTag> RoleRestrictionImpl::get_role() const { return m_role; }

Constructor<ConceptTag> RoleRestrictionImpl::get_concept() const { return m_concept; }

/**
 * RoleIdentity
 */

RoleIdentityImpl::RoleIdentityImpl(Index index, Constructor<ConceptTag> concept_) : m_index(index), m_concept(concept_) {}

void RoleIdentityImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_concept = m_concept->evaluate(context);

    // Fetch data
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();
    auto& bitsets = boost::hana::at_key(context.get_builders(), boost::hana::type<RoleTag> {}).get_data();

    // Compute result
    bitsets.resize(num_objects);
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }
    for (size_t i = 0; i < num_objects; ++i)
    {
        if (eval_concept->get_data().get(i))
        {
            bitsets.at(i).set(i);
        }
    }
}

void RoleIdentityImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t RoleIdentityImpl::get_complexity_impl() const { return m_concept->get_complexity() + 1; }

Index RoleIdentityImpl::get_index() const { return m_index; }

Constructor<ConceptTag> RoleIdentityImpl::get_concept() const { return m_concept; }

/**
 * BooleanAtomicStateImpl
 */

template<IsStaticOrFluentOrDerivedTag P>
BooleanAtomicStateImpl<P>::BooleanAtomicStateImpl(Index index, Predicate<P> predicate) : m_index(index), m_predicate(predicate)
{
}

template<IsStaticOrFluentOrDerivedTag P>
void BooleanAtomicStateImpl<P>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& boolean = boost::hana::at_key(context.get_builders(), boost::hana::type<BooleanTag> {}).get_data();
    boolean = false;

    // Compute result
    for (const auto& atom_index : context.get_state().get_atoms<P>())
    {
        const auto atom = context.get_problem()->get_repositories().template get_ground_atom<P>(atom_index);

        if (atom->get_predicate() == m_predicate)
        {
            boolean = true;
            break;
        }
    }
}

template<>
void BooleanAtomicStateImpl<StaticTag>::evaluate_impl(EvaluationContext& context) const
{
    // Fetch data
    auto& boolean = boost::hana::at_key(context.get_builders(), boost::hana::type<BooleanTag> {}).get_data();
    boolean = false;

    // Compute result
    for (const auto& atom : context.get_problem()->get_static_initial_atoms())
    {
        if (atom->get_predicate() == m_predicate)
        {
            boolean = true;
            break;
        }
    }
}

template<IsStaticOrFluentOrDerivedTag P>
void BooleanAtomicStateImpl<P>::accept_impl(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsStaticOrFluentOrDerivedTag P>
size_t BooleanAtomicStateImpl<P>::get_complexity_impl() const
{
    return 1;
}

template<IsStaticOrFluentOrDerivedTag P>
Index BooleanAtomicStateImpl<P>::get_index() const
{
    return m_index;
}

template<IsStaticOrFluentOrDerivedTag P>
Predicate<P> BooleanAtomicStateImpl<P>::get_predicate() const
{
    return m_predicate;
}

template class BooleanAtomicStateImpl<StaticTag>;
template class BooleanAtomicStateImpl<FluentTag>;
template class BooleanAtomicStateImpl<DerivedTag>;

template<IsConceptOrRoleTag D>
BooleanNonemptyImpl<D>::BooleanNonemptyImpl(Index index, Constructor<D> constructor) : m_index(index), m_constructor(constructor)
{
}

template<IsConceptOrRoleTag D>
void BooleanNonemptyImpl<D>::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_constructor = m_constructor->evaluate(context);

    // Fetch data
    auto& boolean = boost::hana::at_key(context.get_builders(), boost::hana::type<BooleanTag> {}).get_data();

    // Compute result
    boolean = eval_constructor->any();
}

template<IsConceptOrRoleTag D>
void BooleanNonemptyImpl<D>::accept_impl(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsConceptOrRoleTag D>
size_t BooleanNonemptyImpl<D>::get_complexity_impl() const
{
    return m_constructor->get_complexity() + 1;
}

template<IsConceptOrRoleTag D>
Index BooleanNonemptyImpl<D>::get_index() const
{
    return m_index;
}

template<IsConceptOrRoleTag D>
Constructor<D> BooleanNonemptyImpl<D>::get_constructor() const
{
    return m_constructor;
}

template class BooleanNonemptyImpl<ConceptTag>;
template class BooleanNonemptyImpl<RoleTag>;

/**
 * Numericals
 */

template<IsConceptOrRoleTag D>
NumericalCountImpl<D>::NumericalCountImpl(Index index, Constructor<D> constructor) : m_index(index), m_constructor(constructor)
{
}

template<IsConceptOrRoleTag D>
void NumericalCountImpl<D>::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_constructor = m_constructor->evaluate(context);

    // Fetch data
    auto& numerical = boost::hana::at_key(context.get_builders(), boost::hana::type<NumericalTag> {}).get_data();

    // Compute result
    numerical = eval_constructor->count();
}

template<IsConceptOrRoleTag D>
void NumericalCountImpl<D>::accept_impl(IVisitor& visitor) const
{
    visitor.visit(this);
}

template<IsConceptOrRoleTag D>
size_t NumericalCountImpl<D>::get_complexity_impl() const
{
    return m_constructor->get_complexity() + 1;
}

template<IsConceptOrRoleTag D>
Index NumericalCountImpl<D>::get_index() const
{
    return m_index;
}

template<IsConceptOrRoleTag D>
Constructor<D> NumericalCountImpl<D>::get_constructor() const
{
    return m_constructor;
}

template class NumericalCountImpl<ConceptTag>;
template class NumericalCountImpl<RoleTag>;

NumericalDistanceImpl::NumericalDistanceImpl(Index index,
                                             Constructor<ConceptTag> left_concept,
                                             Constructor<RoleTag> role,
                                             Constructor<ConceptTag> right_concept) :
    m_index(index),
    m_left_concept(left_concept),
    m_role(role),
    m_right_concept(right_concept)
{
}

void NumericalDistanceImpl::evaluate_impl(EvaluationContext& context) const
{
    // Evaluate the children
    const auto eval_left_concept = m_left_concept->evaluate(context);
    const auto eval_role = m_role->evaluate(context);
    const auto eval_right_concept = m_right_concept->evaluate(context);

    // Fetch data
    auto& numerical = boost::hana::at_key(context.get_builders(), boost::hana::type<NumericalTag> {}).get_data();
    using DistanceType = typename DenotationImpl<NumericalTag>::DenotationType;
    const auto MAX_DISTANCE = std::numeric_limits<DistanceType>::max();
    numerical = MAX_DISTANCE;
    const auto num_objects = context.get_problem()->get_problem_and_domain_objects().size();

    // Compute result
    if (!eval_left_concept->any() || !eval_right_concept->any())
    {
        return;  ///< there exists no path and the distance is trivially infinity.
    }

    if (!eval_left_concept->get_data().are_disjoint(eval_right_concept->get_data()))
    {
        numerical = 0;
        return;  ///< sources intersects with targets and the distance is trivially zero.
    }

    auto deque = std::deque<Index> {};
    auto distances = std::vector<DistanceType>(num_objects, MAX_DISTANCE);
    for (const auto& object_index : eval_left_concept->get_data())
    {
        deque.push_back(object_index);
        distances[object_index] = 0;
    }

    while (!deque.empty())
    {
        const auto source = deque.front();
        deque.pop_front();

        const auto source_dist = distances[source];
        assert(source_dist != MAX_DISTANCE);

        assert(source < context.get_problem()->get_problem_and_domain_objects().size());

        for (const auto& target : eval_role->get_data().at(source))
        {
            const auto new_dist = source_dist + 1;

            auto& target_dist = distances[target];

            if (new_dist < target_dist)
            {
                target_dist = new_dist;
                deque.push_back(target);
            }

            if (eval_right_concept->get_data().get(target))
            {
                numerical = target_dist;
                return;
            }
        }
    }
}

void NumericalDistanceImpl::accept_impl(IVisitor& visitor) const { visitor.visit(this); }

size_t NumericalDistanceImpl::get_complexity_impl() const
{
    return m_left_concept->get_complexity() + m_role->get_complexity() + m_right_concept->get_complexity() + 1;
}

Index NumericalDistanceImpl::get_index() const { return m_index; }

Constructor<ConceptTag> NumericalDistanceImpl::get_left_concept() const { return m_left_concept; }

Constructor<RoleTag> NumericalDistanceImpl::get_role() const { return m_role; }

Constructor<ConceptTag> NumericalDistanceImpl::get_right_concept() const { return m_right_concept; }

}
