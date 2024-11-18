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

class ConceptBotImpl : public ConstructorEvaluatorBase<Concept, ConceptBotImpl>
{
private:
    Index m_index;

    explicit ConceptBotImpl(Index index);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptBotImpl>;

public:
    // moveable but not copyable
    ConceptBotImpl(const ConceptBotImpl& other) = delete;
    ConceptBotImpl& operator=(const ConceptBotImpl& other) = delete;
    ConceptBotImpl(ConceptBotImpl&& other) = default;
    ConceptBotImpl& operator=(ConceptBotImpl&& other) = default;

    Index get_index() const;
};

class ConceptTopImpl : public ConstructorEvaluatorBase<Concept, ConceptTopImpl>
{
private:
    Index m_index;

    explicit ConceptTopImpl(Index index);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptTopImpl>;

public:
    // moveable but not copyable
    ConceptTopImpl(const ConceptTopImpl& other) = delete;
    ConceptTopImpl& operator=(const ConceptTopImpl& other) = delete;
    ConceptTopImpl(ConceptTopImpl&& other) = default;
    ConceptTopImpl& operator=(ConceptTopImpl&& other) = default;

    Index get_index() const;
};

template<PredicateTag P>
class ConceptAtomicStateImpl : public ConstructorEvaluatorBase<Concept, ConceptAtomicStateImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

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

template<PredicateTag P>
class ConceptAtomicGoalImpl : public ConstructorEvaluatorBase<Concept, ConceptAtomicGoalImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;
    bool m_is_negated;

    ConceptAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptAtomicGoalImpl<P>>;

public:
    // moveable but not copyable
    ConceptAtomicGoalImpl(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl& operator=(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl(ConceptAtomicGoalImpl&& other) = default;
    ConceptAtomicGoalImpl& operator=(ConceptAtomicGoalImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
    bool is_negated() const;
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
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

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
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

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

class ConceptNegationImpl : public ConstructorEvaluatorBase<Concept, ConceptNegationImpl>
{
private:
    Index m_index;
    Constructor<Concept> m_concept;

    ConceptNegationImpl(Index index, Constructor<Concept> concept_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptNegationImpl>;

public:
    // moveable but not copyable
    ConceptNegationImpl(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl& operator=(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl(ConceptNegationImpl&& other) = default;
    ConceptNegationImpl& operator=(ConceptNegationImpl&& other) = default;

    Index get_index() const;
    Constructor<Concept> get_concept() const;
};

class ConceptValueRestrictionImpl : public ConstructorEvaluatorBase<Concept, ConceptValueRestrictionImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;
    Constructor<Concept> m_concept;

    ConceptValueRestrictionImpl(Index index, Constructor<Role> role_, Constructor<Concept> concept_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptValueRestrictionImpl>;

public:
    // moveable but not copyable
    ConceptValueRestrictionImpl(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl& operator=(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl(ConceptValueRestrictionImpl&& other) = default;
    ConceptValueRestrictionImpl& operator=(ConceptValueRestrictionImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
    Constructor<Concept> get_concept() const;
};

class ConceptExistentialQuantificationImpl : public ConstructorEvaluatorBase<Concept, ConceptExistentialQuantificationImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;
    Constructor<Concept> m_concept;

    ConceptExistentialQuantificationImpl(Index index, Constructor<Role> role_, Constructor<Concept> concept_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptExistentialQuantificationImpl>;

public:
    // moveable but not copyable
    ConceptExistentialQuantificationImpl(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl& operator=(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl(ConceptExistentialQuantificationImpl&& other) = default;
    ConceptExistentialQuantificationImpl& operator=(ConceptExistentialQuantificationImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
    Constructor<Concept> get_concept() const;
};

class ConceptRoleValueMapContainmentImpl : public ConstructorEvaluatorBase<Concept, ConceptRoleValueMapContainmentImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role_left;
    Constructor<Role> m_role_right;

    ConceptRoleValueMapContainmentImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptRoleValueMapContainmentImpl>;

public:
    // moveable but not copyable
    ConceptRoleValueMapContainmentImpl(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl& operator=(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl(ConceptRoleValueMapContainmentImpl&& other) = default;
    ConceptRoleValueMapContainmentImpl& operator=(ConceptRoleValueMapContainmentImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role_left() const;
    Constructor<Role> get_role_right() const;
};

class ConceptRoleValueMapEqualityImpl : public ConstructorEvaluatorBase<Concept, ConceptRoleValueMapEqualityImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role_left;
    Constructor<Role> m_role_right;

    ConceptRoleValueMapEqualityImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptRoleValueMapEqualityImpl>;

public:
    // moveable but not copyable
    ConceptRoleValueMapEqualityImpl(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl& operator=(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl(ConceptRoleValueMapEqualityImpl&& other) = default;
    ConceptRoleValueMapEqualityImpl& operator=(ConceptRoleValueMapEqualityImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role_left() const;
    Constructor<Role> get_role_right() const;
};

class ConceptNominalImpl : public ConstructorEvaluatorBase<Concept, ConceptNominalImpl>
{
private:
    Index m_index;
    Object m_object;

    ConceptNominalImpl(Index index, Object object);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Concept, ConceptNominalImpl>;

public:
    // moveable but not copyable
    ConceptNominalImpl(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl& operator=(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl(ConceptNominalImpl&& other) = default;
    ConceptNominalImpl& operator=(ConceptNominalImpl&& other) = default;

    Index get_index() const;
    Object get_object() const;
};

/**
 * Roles
 */

class RoleUniversalImpl : public ConstructorEvaluatorBase<Role, RoleUniversalImpl>
{
private:
    Index m_index;

    explicit RoleUniversalImpl(Index index);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleUniversalImpl>;

public:
    // moveable but not copyable
    RoleUniversalImpl(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl& operator=(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl(RoleUniversalImpl&& other) = default;
    RoleUniversalImpl& operator=(RoleUniversalImpl&& other) = default;

    Index get_index() const;
};

template<PredicateTag P>
class RoleAtomicStateImpl : public ConstructorEvaluatorBase<Role, RoleAtomicStateImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

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

template<PredicateTag P>
class RoleAtomicGoalImpl : public ConstructorEvaluatorBase<Role, RoleAtomicGoalImpl<P>>
{
private:
    Index m_index;
    Predicate<P> m_predicate;
    bool m_is_negated;

    RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleAtomicGoalImpl<P>>;

public:
    // moveable but not copyable
    RoleAtomicGoalImpl(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl& operator=(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl(RoleAtomicGoalImpl&& other) = default;
    RoleAtomicGoalImpl& operator=(RoleAtomicGoalImpl&& other) = default;

    Index get_index() const;
    Predicate<P> get_predicate() const;
    bool is_negated() const;
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
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

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

class RoleUnionImpl : public ConstructorEvaluatorBase<Role, RoleUnionImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role_left;
    Constructor<Role> m_role_right;

    RoleUnionImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleUnionImpl>;

public:
    // moveable but not copyable
    RoleUnionImpl(const RoleUnionImpl& other) = delete;
    RoleUnionImpl& operator=(const RoleUnionImpl& other) = delete;
    RoleUnionImpl(RoleUnionImpl&& other) = default;
    RoleUnionImpl& operator=(RoleUnionImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role_left() const;
    Constructor<Role> get_role_right() const;
};

class RoleComplementImpl : public ConstructorEvaluatorBase<Role, RoleComplementImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;

    RoleComplementImpl(Index index, Constructor<Role> role_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleComplementImpl>;

public:
    // moveable but not copyable
    RoleComplementImpl(const RoleComplementImpl& other) = delete;
    RoleComplementImpl& operator=(const RoleComplementImpl& other) = delete;
    RoleComplementImpl(RoleComplementImpl&& other) = default;
    RoleComplementImpl& operator=(RoleComplementImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
};

class RoleInverseImpl : public ConstructorEvaluatorBase<Role, RoleInverseImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;

    RoleInverseImpl(Index index, Constructor<Role> role_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleInverseImpl>;

public:
    // moveable but not copyable
    RoleInverseImpl(const RoleInverseImpl& other) = delete;
    RoleInverseImpl& operator=(const RoleInverseImpl& other) = delete;
    RoleInverseImpl(RoleInverseImpl&& other) = default;
    RoleInverseImpl& operator=(RoleInverseImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
};

class RoleCompositionImpl : public ConstructorEvaluatorBase<Role, RoleCompositionImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role_left;
    Constructor<Role> m_role_right;

    RoleCompositionImpl(Index index, Constructor<Role> role_left, Constructor<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleCompositionImpl>;

public:
    // moveable but not copyable
    RoleCompositionImpl(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl& operator=(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl(RoleCompositionImpl&& other) = default;
    RoleCompositionImpl& operator=(RoleCompositionImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role_left() const;
    Constructor<Role> get_role_right() const;
};

class RoleTransitiveClosureImpl : public ConstructorEvaluatorBase<Role, RoleTransitiveClosureImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;

    RoleTransitiveClosureImpl(Index index, Constructor<Role> role_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleTransitiveClosureImpl>;

public:
    // moveable but not copyable
    RoleTransitiveClosureImpl(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl& operator=(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl(RoleTransitiveClosureImpl&& other) = default;
    RoleTransitiveClosureImpl& operator=(RoleTransitiveClosureImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
};

class RoleReflexiveTransitiveClosureImpl : public ConstructorEvaluatorBase<Role, RoleReflexiveTransitiveClosureImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;

    RoleReflexiveTransitiveClosureImpl(Index index, Constructor<Role> role_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleReflexiveTransitiveClosureImpl>;

public:
    // moveable but not copyable
    RoleReflexiveTransitiveClosureImpl(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl& operator=(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl(RoleReflexiveTransitiveClosureImpl&& other) = default;
    RoleReflexiveTransitiveClosureImpl& operator=(RoleReflexiveTransitiveClosureImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
};

class RoleRestrictionImpl : public ConstructorEvaluatorBase<Role, RoleRestrictionImpl>
{
private:
    Index m_index;
    Constructor<Role> m_role;
    Constructor<Concept> m_concept;

    RoleRestrictionImpl(Index index, Constructor<Role> role_, Constructor<Concept> concept_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleRestrictionImpl>;

public:
    // moveable but not copyable
    RoleRestrictionImpl(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl& operator=(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl(RoleRestrictionImpl&& other) = default;
    RoleRestrictionImpl& operator=(RoleRestrictionImpl&& other) = default;

    Index get_index() const;
    Constructor<Role> get_role() const;
    Constructor<Concept> get_concept() const;
};

class RoleIdentityImpl : public ConstructorEvaluatorBase<Role, RoleIdentityImpl>
{
private:
    Index m_index;
    Constructor<Concept> m_concept;

    RoleIdentityImpl(Index index, Constructor<Concept> concept_);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(Visitor& visitor) const;

    friend class ConstructorEvaluatorBase<Role, RoleIdentityImpl>;

public:
    // moveable but not copyable
    RoleIdentityImpl(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl& operator=(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl(RoleIdentityImpl&& other) = default;
    RoleIdentityImpl& operator=(RoleIdentityImpl&& other) = default;

    Index get_index() const;
    Constructor<Concept> get_concept() const;
};

}

#endif
