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

namespace mimir::languages::dl
{

/**
 * Concepts
 */

class ConceptBotImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptBotImpl>
{
private:
    Index m_index;

    explicit ConceptBotImpl(Index index);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptBotImpl>;

public:
    // moveable but not copyable
    ConceptBotImpl(const ConceptBotImpl& other) = delete;
    ConceptBotImpl& operator=(const ConceptBotImpl& other) = delete;
    ConceptBotImpl(ConceptBotImpl&& other) = default;
    ConceptBotImpl& operator=(ConceptBotImpl&& other) = default;

    Index get_index() const override;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(); }
};

class ConceptTopImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptTopImpl>
{
private:
    Index m_index;

    explicit ConceptTopImpl(Index index);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptTopImpl>;

public:
    // moveable but not copyable
    ConceptTopImpl(const ConceptTopImpl& other) = delete;
    ConceptTopImpl& operator=(const ConceptTopImpl& other) = delete;
    ConceptTopImpl(ConceptTopImpl&& other) = default;
    ConceptTopImpl& operator=(ConceptTopImpl&& other) = default;

    Index get_index() const override;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicStateImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptAtomicStateImpl<P>>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptAtomicStateImpl<P>>;

public:
    // moveable but not copyable
    ConceptAtomicStateImpl(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl& operator=(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl(ConceptAtomicStateImpl&& other) = default;
    ConceptAtomicStateImpl& operator=(ConceptAtomicStateImpl&& other) = default;

    Index get_index() const override;
    formalism::Predicate<P> get_predicate() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate()); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicGoalImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptAtomicGoalImpl<P>>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;
    bool m_polarity;

    ConceptAtomicGoalImpl(Index index, formalism::Predicate<P> predicate, bool polarity);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptAtomicGoalImpl<P>>;

public:
    // moveable but not copyable
    ConceptAtomicGoalImpl(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl& operator=(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl(ConceptAtomicGoalImpl&& other) = default;
    ConceptAtomicGoalImpl& operator=(ConceptAtomicGoalImpl&& other) = default;

    Index get_index() const override;
    formalism::Predicate<P> get_predicate() const;
    bool get_polarity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate(), get_polarity()); }
};

class ConceptIntersectionImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptIntersectionImpl>
{
private:
    Index m_index;
    Constructor<ConceptTag> m_left_concept;
    Constructor<ConceptTag> m_right_concept;

    ConceptIntersectionImpl(Index index, Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptIntersectionImpl>;

public:
    // moveable but not copyable
    ConceptIntersectionImpl(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl& operator=(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl(ConceptIntersectionImpl&& other) = default;
    ConceptIntersectionImpl& operator=(ConceptIntersectionImpl&& other) = default;

    Index get_index() const override;
    Constructor<ConceptTag> get_left_concept() const;
    Constructor<ConceptTag> get_right_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_concept(), get_right_concept()); }
};

class ConceptUnionImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptUnionImpl>
{
private:
    Index m_index;
    Constructor<ConceptTag> m_left_concept;
    Constructor<ConceptTag> m_right_concept;

    ConceptUnionImpl(Index index, Constructor<ConceptTag> left_concept, Constructor<ConceptTag> right_concept);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptUnionImpl>;

public:
    // moveable but not copyable
    ConceptUnionImpl(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl& operator=(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl(ConceptUnionImpl&& other) = default;
    ConceptUnionImpl& operator=(ConceptUnionImpl&& other) = default;

    Index get_index() const override;
    Constructor<ConceptTag> get_left_concept() const;
    Constructor<ConceptTag> get_right_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_concept(), get_right_concept()); }
};

class ConceptNegationImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptNegationImpl>
{
private:
    Index m_index;
    Constructor<ConceptTag> m_concept;

    ConceptNegationImpl(Index index, Constructor<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptNegationImpl>;

public:
    // moveable but not copyable
    ConceptNegationImpl(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl& operator=(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl(ConceptNegationImpl&& other) = default;
    ConceptNegationImpl& operator=(ConceptNegationImpl&& other) = default;

    Index get_index() const override;
    Constructor<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_concept()); }
};

class ConceptValueRestrictionImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptValueRestrictionImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;
    Constructor<ConceptTag> m_concept;

    ConceptValueRestrictionImpl(Index index, Constructor<RoleTag> role_, Constructor<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptValueRestrictionImpl>;

public:
    // moveable but not copyable
    ConceptValueRestrictionImpl(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl& operator=(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl(ConceptValueRestrictionImpl&& other) = default;
    ConceptValueRestrictionImpl& operator=(ConceptValueRestrictionImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;
    Constructor<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role(), get_concept()); }
};

class ConceptExistentialQuantificationImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptExistentialQuantificationImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;
    Constructor<ConceptTag> m_concept;

    ConceptExistentialQuantificationImpl(Index index, Constructor<RoleTag> role_, Constructor<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptExistentialQuantificationImpl>;

public:
    // moveable but not copyable
    ConceptExistentialQuantificationImpl(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl& operator=(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl(ConceptExistentialQuantificationImpl&& other) = default;
    ConceptExistentialQuantificationImpl& operator=(ConceptExistentialQuantificationImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;
    Constructor<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role(), get_concept()); }
};

class ConceptRoleValueMapContainmentImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptRoleValueMapContainmentImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_left_role;
    Constructor<RoleTag> m_right_role;

    ConceptRoleValueMapContainmentImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptRoleValueMapContainmentImpl>;

public:
    // moveable but not copyable
    ConceptRoleValueMapContainmentImpl(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl& operator=(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl(ConceptRoleValueMapContainmentImpl&& other) = default;
    ConceptRoleValueMapContainmentImpl& operator=(ConceptRoleValueMapContainmentImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_left_role() const;
    Constructor<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class ConceptRoleValueMapEqualityImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptRoleValueMapEqualityImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_left_role;
    Constructor<RoleTag> m_right_role;

    ConceptRoleValueMapEqualityImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptRoleValueMapEqualityImpl>;

public:
    // moveable but not copyable
    ConceptRoleValueMapEqualityImpl(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl& operator=(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl(ConceptRoleValueMapEqualityImpl&& other) = default;
    ConceptRoleValueMapEqualityImpl& operator=(ConceptRoleValueMapEqualityImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_left_role() const;
    Constructor<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class ConceptNominalImpl : public ConstructorEvaluatorBase<ConceptTag, ConceptNominalImpl>
{
private:
    Index m_index;
    formalism::Object m_object;

    ConceptNominalImpl(Index index, formalism::Object object);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<ConceptTag, ConceptNominalImpl>;

public:
    // moveable but not copyable
    ConceptNominalImpl(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl& operator=(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl(ConceptNominalImpl&& other) = default;
    ConceptNominalImpl& operator=(ConceptNominalImpl&& other) = default;

    Index get_index() const override;
    formalism::Object get_object() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_object()); }
};

/**
 * Roles
 */

class RoleUniversalImpl : public ConstructorEvaluatorBase<RoleTag, RoleUniversalImpl>
{
private:
    Index m_index;

    explicit RoleUniversalImpl(Index index);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleUniversalImpl>;

public:
    // moveable but not copyable
    RoleUniversalImpl(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl& operator=(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl(RoleUniversalImpl&& other) = default;
    RoleUniversalImpl& operator=(RoleUniversalImpl&& other) = default;

    Index get_index() const override;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicStateImpl : public ConstructorEvaluatorBase<RoleTag, RoleAtomicStateImpl<P>>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleAtomicStateImpl<P>>;

public:
    // moveable but not copyable
    RoleAtomicStateImpl(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl& operator=(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl(RoleAtomicStateImpl&& other) = default;
    RoleAtomicStateImpl& operator=(RoleAtomicStateImpl&& other) = default;

    Index get_index() const override;
    formalism::Predicate<P> get_predicate() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate()); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicGoalImpl : public ConstructorEvaluatorBase<RoleTag, RoleAtomicGoalImpl<P>>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;
    bool m_polarity;

    RoleAtomicGoalImpl(Index index, formalism::Predicate<P> predicate, bool polarity);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleAtomicGoalImpl<P>>;

public:
    // moveable but not copyable
    RoleAtomicGoalImpl(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl& operator=(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl(RoleAtomicGoalImpl&& other) = default;
    RoleAtomicGoalImpl& operator=(RoleAtomicGoalImpl&& other) = default;

    Index get_index() const override;
    formalism::Predicate<P> get_predicate() const;
    bool get_polarity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate(), get_polarity()); }
};

class RoleIntersectionImpl : public ConstructorEvaluatorBase<RoleTag, RoleIntersectionImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_left_role;
    Constructor<RoleTag> m_right_role;

    RoleIntersectionImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleIntersectionImpl>;

public:
    // moveable but not copyable
    RoleIntersectionImpl(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl& operator=(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl(RoleIntersectionImpl&& other) = default;
    RoleIntersectionImpl& operator=(RoleIntersectionImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_left_role() const;
    Constructor<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class RoleUnionImpl : public ConstructorEvaluatorBase<RoleTag, RoleUnionImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_left_role;
    Constructor<RoleTag> m_right_role;

    RoleUnionImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleUnionImpl>;

public:
    // moveable but not copyable
    RoleUnionImpl(const RoleUnionImpl& other) = delete;
    RoleUnionImpl& operator=(const RoleUnionImpl& other) = delete;
    RoleUnionImpl(RoleUnionImpl&& other) = default;
    RoleUnionImpl& operator=(RoleUnionImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_left_role() const;
    Constructor<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class RoleComplementImpl : public ConstructorEvaluatorBase<RoleTag, RoleComplementImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;

    RoleComplementImpl(Index index, Constructor<RoleTag> role_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleComplementImpl>;

public:
    // moveable but not copyable
    RoleComplementImpl(const RoleComplementImpl& other) = delete;
    RoleComplementImpl& operator=(const RoleComplementImpl& other) = delete;
    RoleComplementImpl(RoleComplementImpl&& other) = default;
    RoleComplementImpl& operator=(RoleComplementImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleInverseImpl : public ConstructorEvaluatorBase<RoleTag, RoleInverseImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;

    RoleInverseImpl(Index index, Constructor<RoleTag> role_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleInverseImpl>;

public:
    // moveable but not copyable
    RoleInverseImpl(const RoleInverseImpl& other) = delete;
    RoleInverseImpl& operator=(const RoleInverseImpl& other) = delete;
    RoleInverseImpl(RoleInverseImpl&& other) = default;
    RoleInverseImpl& operator=(RoleInverseImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleCompositionImpl : public ConstructorEvaluatorBase<RoleTag, RoleCompositionImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_left_role;
    Constructor<RoleTag> m_right_role;

    RoleCompositionImpl(Index index, Constructor<RoleTag> left_role, Constructor<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleCompositionImpl>;

public:
    // moveable but not copyable
    RoleCompositionImpl(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl& operator=(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl(RoleCompositionImpl&& other) = default;
    RoleCompositionImpl& operator=(RoleCompositionImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_left_role() const;
    Constructor<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class RoleTransitiveClosureImpl : public ConstructorEvaluatorBase<RoleTag, RoleTransitiveClosureImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;

    RoleTransitiveClosureImpl(Index index, Constructor<RoleTag> role_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleTransitiveClosureImpl>;

public:
    // moveable but not copyable
    RoleTransitiveClosureImpl(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl& operator=(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl(RoleTransitiveClosureImpl&& other) = default;
    RoleTransitiveClosureImpl& operator=(RoleTransitiveClosureImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleReflexiveTransitiveClosureImpl : public ConstructorEvaluatorBase<RoleTag, RoleReflexiveTransitiveClosureImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;

    RoleReflexiveTransitiveClosureImpl(Index index, Constructor<RoleTag> role_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleReflexiveTransitiveClosureImpl>;

public:
    // moveable but not copyable
    RoleReflexiveTransitiveClosureImpl(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl& operator=(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl(RoleReflexiveTransitiveClosureImpl&& other) = default;
    RoleReflexiveTransitiveClosureImpl& operator=(RoleReflexiveTransitiveClosureImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleRestrictionImpl : public ConstructorEvaluatorBase<RoleTag, RoleRestrictionImpl>
{
private:
    Index m_index;
    Constructor<RoleTag> m_role;
    Constructor<ConceptTag> m_concept;

    RoleRestrictionImpl(Index index, Constructor<RoleTag> role_, Constructor<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleRestrictionImpl>;

public:
    // moveable but not copyable
    RoleRestrictionImpl(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl& operator=(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl(RoleRestrictionImpl&& other) = default;
    RoleRestrictionImpl& operator=(RoleRestrictionImpl&& other) = default;

    Index get_index() const override;
    Constructor<RoleTag> get_role() const;
    Constructor<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role(), get_concept()); }
};

class RoleIdentityImpl : public ConstructorEvaluatorBase<RoleTag, RoleIdentityImpl>
{
private:
    Index m_index;
    Constructor<ConceptTag> m_concept;

    RoleIdentityImpl(Index index, Constructor<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<RoleTag, RoleIdentityImpl>;

public:
    // moveable but not copyable
    RoleIdentityImpl(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl& operator=(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl(RoleIdentityImpl&& other) = default;
    RoleIdentityImpl& operator=(RoleIdentityImpl&& other) = default;

    Index get_index() const override;
    Constructor<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_concept()); }
};

/**
 * Booleans
 */

template<formalism::IsStaticOrFluentOrDerivedTag P>
class BooleanAtomicStateImpl : public ConstructorEvaluatorBase<BooleanTag, BooleanAtomicStateImpl<P>>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    BooleanAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<BooleanTag, BooleanAtomicStateImpl<P>>;

public:
    // moveable but not copyable
    BooleanAtomicStateImpl(const BooleanAtomicStateImpl& other) = delete;
    BooleanAtomicStateImpl& operator=(const BooleanAtomicStateImpl& other) = delete;
    BooleanAtomicStateImpl(BooleanAtomicStateImpl&& other) = default;
    BooleanAtomicStateImpl& operator=(BooleanAtomicStateImpl&& other) = default;

    Index get_index() const;
    formalism::Predicate<P> get_predicate() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate()); }
};

template<IsConceptOrRoleTag D>
class BooleanNonemptyImpl : public ConstructorEvaluatorBase<BooleanTag, BooleanNonemptyImpl<D>>
{
private:
    Index m_index;
    Constructor<D> m_constructor;

    BooleanNonemptyImpl(Index index, Constructor<D> constructor);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<BooleanTag, BooleanNonemptyImpl<D>>;

public:
    // moveable but not copyable
    BooleanNonemptyImpl(const BooleanNonemptyImpl& other) = delete;
    BooleanNonemptyImpl& operator=(const BooleanNonemptyImpl& other) = delete;
    BooleanNonemptyImpl(BooleanNonemptyImpl&& other) = default;
    BooleanNonemptyImpl& operator=(BooleanNonemptyImpl&& other) = default;

    Index get_index() const;
    Constructor<D> get_constructor() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_constructor()); }
};

/**
 * Numericals
 */

template<IsConceptOrRoleTag D>
class NumericalCountImpl : public ConstructorEvaluatorBase<NumericalTag, NumericalCountImpl<D>>
{
private:
    Index m_index;
    Constructor<D> m_constructor;

    NumericalCountImpl(Index index, Constructor<D> constructor);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<NumericalTag, NumericalCountImpl<D>>;

public:
    // moveable but not copyable
    NumericalCountImpl(const NumericalCountImpl& other) = delete;
    NumericalCountImpl& operator=(const NumericalCountImpl& other) = delete;
    NumericalCountImpl(NumericalCountImpl&& other) = default;
    NumericalCountImpl& operator=(NumericalCountImpl&& other) = default;

    Index get_index() const;
    Constructor<D> get_constructor() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_constructor()); }
};

class NumericalDistanceImpl : public ConstructorEvaluatorBase<NumericalTag, NumericalDistanceImpl>
{
private:
    Index m_index;
    Constructor<ConceptTag> m_left_concept;
    Constructor<RoleTag> m_role;
    Constructor<ConceptTag> m_right_concept;

    NumericalDistanceImpl(Index index, Constructor<ConceptTag> left_concept, Constructor<RoleTag> role, Constructor<ConceptTag> right_concept);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

    void evaluate_impl(EvaluationContext& context) const;

    void accept_impl(IVisitor& visitor) const;

    friend class ConstructorEvaluatorBase<NumericalTag, NumericalDistanceImpl>;

public:
    // moveable but not copyable
    NumericalDistanceImpl(const NumericalDistanceImpl& other) = delete;
    NumericalDistanceImpl& operator=(const NumericalDistanceImpl& other) = delete;
    NumericalDistanceImpl(NumericalDistanceImpl&& other) = default;
    NumericalDistanceImpl& operator=(NumericalDistanceImpl&& other) = default;

    Index get_index() const;
    Constructor<ConceptTag> get_left_concept() const;
    Constructor<RoleTag> get_role() const;
    Constructor<ConceptTag> get_right_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_concept(), get_role(), get_right_concept()); }
};

}

#endif
