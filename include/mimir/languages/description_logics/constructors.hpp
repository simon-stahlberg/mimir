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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_HPP_

#include "mimir/languages/description_logics/constructor_base.hpp"
#include "mimir/languages/description_logics/evaluation_context.hpp"
#include "mimir/languages/description_logics/grammar_visitors_interface.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * Forward declarations
 */

template<typename T>
class ConstructorRepository;

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public ConceptConstructorEvaluatorBase<ConceptPredicateState<P>>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

    bool is_equal_impl(const Constructor<Concept>& other) const;
    size_t hash_impl() const;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const ConceptVisitor& visitor) const;

    size_t get_id_impl() const;

    friend class ConceptConstructorEvaluatorBase<ConceptPredicateState>;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateState(const ConceptPredicateState& other) = delete;
    ConceptPredicateState& operator=(const ConceptPredicateState& other) = delete;
    ConceptPredicateState(ConceptPredicateState&& other) = default;
    ConceptPredicateState& operator=(ConceptPredicateState&& other) = default;

    bool operator==(const ConceptPredicateState& other) const;

    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateGoal(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal& operator=(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal(ConceptPredicateGoal&& other) = default;
    ConceptPredicateGoal& operator=(ConceptPredicateGoal&& other) = default;

    bool operator==(const ConceptPredicateGoal& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    Denotation<Concept> evaluate(EvaluationContext& context) const override;

    bool accept(const ConceptVisitor& visitor) const override;

    size_t get_id() const override;

    Predicate<P> get_predicate() const;
};

class ConceptAnd : public Constructor<Concept>
{
private:
    size_t m_id;
    const Constructor<Concept>& m_concept_left;
    const Constructor<Concept>& m_concept_right;

    ConceptAnd(size_t id, const Constructor<Concept>& concept_left, const Constructor<Concept>& concept_right);

    template<typename T>
    friend class ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptAnd(const ConceptAnd& other) = delete;
    ConceptAnd& operator=(const ConceptAnd& other) = delete;
    ConceptAnd(ConceptAnd&& other) = default;
    ConceptAnd& operator=(ConceptAnd&& other) = default;

    bool operator==(const ConceptAnd& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    Denotation<Concept> evaluate(EvaluationContext& context) const override;

    bool accept(const ConceptVisitor& visitor) const override;

    size_t get_id() const override;

    const Constructor<Concept>& get_concept_left() const;
    const Constructor<Concept>& get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateState : public Constructor<Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RolePredicateState(const RolePredicateState& other) = delete;
    RolePredicateState& operator=(const RolePredicateState& other) = delete;
    RolePredicateState(RolePredicateState&& other) = default;
    RolePredicateState& operator=(RolePredicateState&& other) = default;

    bool operator==(const RolePredicateState& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    Denotation<Role> evaluate(EvaluationContext& context) const override;

    bool accept(const RoleVisitor& visitor) const override;

    size_t get_id() const override;

    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoal : public Constructor<Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    RolePredicateGoal(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RolePredicateGoal(const RolePredicateGoal& other) = delete;
    RolePredicateGoal& operator=(const RolePredicateGoal& other) = delete;
    RolePredicateGoal(RolePredicateGoal&& other) = default;
    RolePredicateGoal& operator=(RolePredicateGoal&& other) = default;

    bool operator==(const RolePredicateGoal& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    Denotation<Role> evaluate(EvaluationContext& context) const override;

    bool accept(const RoleVisitor& visitor) const override;

    size_t get_id() const override;

    Predicate<P> get_predicate() const;
};

class RoleAnd : public Constructor<Role>
{
private:
    size_t m_id;
    const Constructor<Role>& m_role_left;
    const Constructor<Role>& m_role_right;

    RoleAnd(size_t id, const Constructor<Role>& role_left, const Constructor<Role>& role_right);

    template<typename T>
    friend class ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RoleAnd(const RoleAnd& other) = delete;
    RoleAnd& operator=(const RoleAnd& other) = delete;
    RoleAnd(RoleAnd&& other) = default;
    RoleAnd& operator=(RoleAnd&& other) = default;

    bool operator==(const RoleAnd& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    Denotation<Role> evaluate(EvaluationContext& context) const override;

    bool accept(const RoleVisitor& visitor) const override;

    size_t get_id() const override;

    const Constructor<Role>& get_role_left() const;
    const Constructor<Role>& get_role_right() const;
};

/**
 * Implementations
 */

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
    auto& bitset = context.concept_denotation.get_bitset();
    for (const auto& atom : context.factories.get().get_ground_atoms_from_ids<P>(context.state.get_atoms<P>()))
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_objects().at(0).get_identifier());
        }
    }
}

template<>
inline void ConceptPredicateState<Static>::evaluate_impl(EvaluationContext& context) const
{
    auto& bitset = context.concept_denotation.get_bitset();
    for (const auto& atom : context.factories.get().get_ground_atoms_from_ids<Static>(context.problem->get_static_initial_positive_atoms_bitset()))
    {
        if (atom->get_predicate() == m_predicate)
        {
            bitset.set(atom->get_identifier());
        }
    }
}

template<PredicateCategory P>
bool ConceptPredicateState<P>::accept_impl(const ConceptVisitor& visitor) const
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
Denotation<Concept> ConceptPredicateGoal<P>::evaluate(EvaluationContext& context) const
{
    // Try to access cached result
    auto denotation = context.concept_denotation_repository.get_if(this);
    if (denotation.has_value())
    {
        return denotation.value();
    }

    // Fetch data
    auto& bitset = context.concept_denotation.get_bitset();
    bitset.unset_all();

    // Compute result
    for (const auto& literal : context.problem->get_goal_condition<P>())
    {
        if (literal->get_atom()->get_predicate() == m_predicate && !literal->is_negated())
        {
            bitset.set(literal->get_atom()->get_objects().at(0)->get_identifier());
        }
    }

    // Store and return result;
    context.concept_denotation.get_flatmemory_builder().finish();
    return context.concept_denotation_repository.insert(this, context.concept_denotation);
}

template<PredicateCategory P>
bool ConceptPredicateGoal<P>::accept(const ConceptVisitor& visitor) const
{
    return visitor.visit(*this);
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
Denotation<Role> RolePredicateState<P>::evaluate(EvaluationContext& context) const
{
    // Try to access cached result
    auto denotation = context.role_denotation_repository.get_if(this, context.state);
    if (denotation.has_value())
    {
        return denotation.value();
    }

    // Fetch data
    auto& bitsets = context.role_denotation.get_bitsets();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& atom : context.factories.get().get_ground_atoms_from_ids<P>(context.state.get_atoms<P>()))
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_identifier();
            const auto object_right_id = atom->get_objects().at(1)->get_identifier();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }

    // Store and return result;
    context.role_denotation.get_flatmemory_builder().finish();
    return context.role_denotation_repository.insert(this, context.state, context.role_denotation);
}

template<>
inline Denotation<Role> RolePredicateState<Static>::evaluate(EvaluationContext& context) const
{
    // Try to access cached result
    auto denotation = context.role_denotation_repository.get_if(this);
    if (denotation.has_value())
    {
        return denotation.value();
    }

    // Fetch data
    auto& bitsets = context.role_denotation.get_bitsets();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& atom : context.factories.get().get_ground_atoms_from_ids<Static>(context.problem->get_static_initial_positive_atoms_bitset()))
    {
        if (atom->get_predicate() == m_predicate)
        {
            const auto object_left_id = atom->get_objects().at(0)->get_identifier();
            const auto object_right_id = atom->get_objects().at(1)->get_identifier();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }

    // Store and return result;
    context.role_denotation.get_flatmemory_builder().finish();
    return context.role_denotation_repository.insert(this, context.role_denotation);
}

template<PredicateCategory P>
bool RolePredicateState<P>::accept(const RoleVisitor& visitor) const
{
    return visitor.visit(*this);
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
Denotation<Role> RolePredicateGoal<P>::evaluate(EvaluationContext& context) const
{
    // Try to access cached result
    auto denotation = context.role_denotation_repository.get_if(this);
    if (denotation.has_value())
    {
        return denotation.value();
    }

    // Fetch data
    auto& bitsets = context.role_denotation.get_bitsets();
    for (auto& bitset : bitsets)
    {
        bitset.unset_all();
    }

    // Compute result
    for (const auto& literal : context.problem->get_goal_condition<P>())
    {
        if (literal->get_atom()->get_predicate() == m_predicate && !literal->is_negated())
        {
            const auto object_left_id = literal->get_atom()->get_objects().at(0)->get_identifier();
            const auto object_right_id = literal->get_atom()->get_objects().at(1)->get_identifier();
            bitsets.at(object_left_id).set(object_right_id);
        }
    }

    // Store and return result;
    context.role_denotation.get_flatmemory_builder().finish();
    return context.role_denotation_repository.insert(this, context.role_denotation);
}

template<PredicateCategory P>
bool RolePredicateGoal<P>::accept(const RoleVisitor& visitor) const
{
    return visitor.visit(*this);
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

}

#endif
