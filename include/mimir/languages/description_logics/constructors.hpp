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
class ConceptAtomicStateImpl : public ConstructorEvaluatorBase<Concept, ConceptAtomicStateImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptAtomicStateImpl<P>>;

public:
    // moveable but not copyable
    ConceptAtomicStateImpl(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl& operator=(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl(ConceptAtomicStateImpl&& other) = default;
    ConceptAtomicStateImpl& operator=(ConceptAtomicStateImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptAtomicGoalImpl : public ConstructorEvaluatorBase<Concept, ConceptAtomicGoalImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    ConceptAtomicGoalImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptAtomicGoalImpl<P>>;

public:
    // moveable but not copyable
    ConceptAtomicGoalImpl(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl& operator=(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl(ConceptAtomicGoalImpl&& other) = default;
    ConceptAtomicGoalImpl& operator=(ConceptAtomicGoalImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

class ConceptIntersectionImpl : public ConstructorEvaluatorBase<Concept, ConceptIntersectionImpl>
{
private:
    Index m_index;
    Constructor<Concept> m_concept_left;
    Constructor<Concept> m_concept_right;

    ConceptIntersectionImpl(Index index, Constructor<Concept> concept_left, Constructor<Concept> concept_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptIntersectionImpl>;

public:
    // moveable but not copyable
    ConceptIntersectionImpl(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl& operator=(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl(ConceptIntersectionImpl&& other) = default;
    ConceptIntersectionImpl& operator=(ConceptIntersectionImpl&& other) = default;

    Index get_index() const;
    Constructor<Concept> get_concept_left() const;
    Constructor<Concept> get_concept_right() const;
};

class ConceptUnionImpl : public ConstructorEvaluatorBase<Concept, ConceptUnionImpl>
{
private:
    Index m_index;
    Constructor<Concept> m_concept_left;
    Constructor<Concept> m_concept_right;

    ConceptUnionImpl(Index index, Constructor<Concept> concept_left, Constructor<Concept> concept_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Concept>& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptUnionImpl>;

public:
    // moveable but not copyable
    ConceptUnionImpl(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl& operator=(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl(ConceptUnionImpl&& other) = default;
    ConceptUnionImpl& operator=(ConceptUnionImpl&& other) = default;

    Index get_index() const;
    Constructor<Concept> get_concept_left() const;
    Constructor<Concept> get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RoleAtomicStateImpl : public ConstructorEvaluatorBase<Role, RoleAtomicStateImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleAtomicStateImpl<P>>;

public:
    // moveable but not copyable
    RoleAtomicStateImpl(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl& operator=(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl(RoleAtomicStateImpl&& other) = default;
    RoleAtomicStateImpl& operator=(RoleAtomicStateImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RoleAtomicGoalImpl : public ConstructorEvaluatorBase<Role, RoleAtomicGoalImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    RoleAtomicGoalImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleAtomicGoalImpl<P>>;

public:
    // moveable but not copyable
    RoleAtomicGoalImpl(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl& operator=(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl(RoleAtomicGoalImpl&& other) = default;
    RoleAtomicGoalImpl& operator=(RoleAtomicGoalImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

class RoleIntersectionImpl : public ConstructorEvaluatorBase<Role, RoleIntersectionImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role_left;
    Constructor<Role> m_role_right;

    RoleIntersectionImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

    void evaluate_impl(EvaluationContext& context) const;

    bool accept_impl(const grammar::Visitor<Role>& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleIntersectionImpl>;

public:
    // moveable but not copyable
    RoleIntersectionImpl(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl& operator=(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl(RoleIntersectionImpl&& other) = default;
    RoleIntersectionImpl& operator=(RoleIntersectionImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role_left() const;
    Constructor<Role> get_role_right() const;
};

}

#endif
