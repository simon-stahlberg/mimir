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

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/constructor_base.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateStateImpl : public ConstructorEvaluatorBase<Concept, ConceptPredicateStateImpl<P>>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    ConceptPredicateStateImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptPredicateStateImpl<P>>;

public:
    // moveable but not copyable
    ConceptPredicateStateImpl(const ConceptPredicateStateImpl& other) = delete;
    ConceptPredicateStateImpl& operator=(const ConceptPredicateStateImpl& other) = delete;
    ConceptPredicateStateImpl(ConceptPredicateStateImpl&& other) = default;
    ConceptPredicateStateImpl& operator=(ConceptPredicateStateImpl&& other) = default;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoalImpl : public ConstructorEvaluatorBase<Concept, ConceptPredicateGoalImpl<P>>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    ConceptPredicateGoalImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptPredicateGoalImpl<P>>;

public:
    // moveable but not copyable
    ConceptPredicateGoalImpl(const ConceptPredicateGoalImpl& other) = delete;
    ConceptPredicateGoalImpl& operator=(const ConceptPredicateGoalImpl& other) = delete;
    ConceptPredicateGoalImpl(ConceptPredicateGoalImpl&& other) = default;
    ConceptPredicateGoalImpl& operator=(ConceptPredicateGoalImpl&& other) = default;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

class ConceptAndImpl : public ConstructorEvaluatorBase<Concept, ConceptAndImpl>
{
private:
    size_t m_index;
    Constructor<Concept> m_concept_left;
    Constructor<Concept> m_concept_right;

    ConceptAndImpl(size_t index, Constructor<Concept> concept_left, Constructor<Concept> concept_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptAndImpl>;

public:
    // moveable but not copyable
    ConceptAndImpl(const ConceptAndImpl& other) = delete;
    ConceptAndImpl& operator=(const ConceptAndImpl& other) = delete;
    ConceptAndImpl(ConceptAndImpl&& other) = default;
    ConceptAndImpl& operator=(ConceptAndImpl&& other) = default;

    size_t get_index() const;
    Constructor<Concept> get_concept_left() const;
    Constructor<Concept> get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateStateImpl : public ConstructorEvaluatorBase<Role, RolePredicateStateImpl<P>>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    RolePredicateStateImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RolePredicateStateImpl<P>>;

public:
    // moveable but not copyable
    RolePredicateStateImpl(const RolePredicateStateImpl& other) = delete;
    RolePredicateStateImpl& operator=(const RolePredicateStateImpl& other) = delete;
    RolePredicateStateImpl(RolePredicateStateImpl&& other) = default;
    RolePredicateStateImpl& operator=(RolePredicateStateImpl&& other) = default;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoalImpl : public ConstructorEvaluatorBase<Role, RolePredicateGoalImpl<P>>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    RolePredicateGoalImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RolePredicateGoalImpl<P>>;

public:
    // moveable but not copyable
    RolePredicateGoalImpl(const RolePredicateGoalImpl& other) = delete;
    RolePredicateGoalImpl& operator=(const RolePredicateGoalImpl& other) = delete;
    RolePredicateGoalImpl(RolePredicateGoalImpl&& other) = default;
    RolePredicateGoalImpl& operator=(RolePredicateGoalImpl&& other) = default;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

class RoleAndImpl : public ConstructorEvaluatorBase<Role, RoleAndImpl>
{
private:
    size_t m_index;
    Constructor<Role> m_role_left;
    Constructor<Role> m_role_right;

    RoleAndImpl(size_t index, Constructor<Role> role_left, Constructor<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleAndImpl>;

public:
    // moveable but not copyable
    RoleAndImpl(const RoleAndImpl& other) = delete;
    RoleAndImpl& operator=(const RoleAndImpl& other) = delete;
    RoleAndImpl(RoleAndImpl&& other) = default;
    RoleAndImpl& operator=(RoleAndImpl&& other) = default;

    size_t get_index() const;
    Constructor<Role> get_role_left() const;
    Constructor<Role> get_role_right() const;
};

}

#endif
