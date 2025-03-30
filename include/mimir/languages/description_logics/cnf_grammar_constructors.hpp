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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CNF_GRAMMAR_CONSTRUCTORS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/cnf_grammar_constructor_interface.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/tags.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace mimir::languages::dl::cnf_grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class NonTerminalImpl
{
private:
    Index m_index;
    // Use name for equality and hash since rule is deferred instantiated.
    std::string m_name;

    NonTerminalImpl(Index index, std::string name);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NonTerminalImpl(const NonTerminalImpl& other) = delete;
    NonTerminalImpl& operator=(const NonTerminalImpl& other) = delete;
    NonTerminalImpl(NonTerminalImpl&& other) = default;
    NonTerminalImpl& operator=(NonTerminalImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor, const Grammar& grammar) const;

    void accept(IVisitor& visitor) const;

    Index get_index() const;
    const std::string& get_name() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(std::cref(get_name())); }
};

/**
 * DerivationRule
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class DerivationRuleImpl
{
private:
    Index m_index;
    NonTerminal<D> m_head;
    Constructor<D> m_body;

    DerivationRuleImpl(Index index, NonTerminal<D> head, Constructor<D> body);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    DerivationRuleImpl(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl& operator=(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl(DerivationRuleImpl&& other) = default;
    DerivationRuleImpl& operator=(DerivationRuleImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor, const Grammar& grammar) const;

    void accept(IVisitor& visitor) const;

    Index get_index() const;
    const NonTerminal<D>& get_head() const;
    const Constructor<D>& get_body() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_head(), get_body()); }
};

/**
 * SubstitutionRule
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class SubstitutionRuleImpl
{
private:
    Index m_index;
    NonTerminal<D> m_head;
    NonTerminal<D> m_body;

    SubstitutionRuleImpl(Index index, NonTerminal<D> head, NonTerminal<D> body);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    SubstitutionRuleImpl(const SubstitutionRuleImpl& other) = delete;
    SubstitutionRuleImpl& operator=(const SubstitutionRuleImpl& other) = delete;
    SubstitutionRuleImpl(SubstitutionRuleImpl&& other) = default;
    SubstitutionRuleImpl& operator=(SubstitutionRuleImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor, const Grammar& grammar) const;

    void accept(IVisitor& visitor) const;

    Index get_index() const;
    const NonTerminal<D>& get_head() const;
    const NonTerminal<D>& get_body() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_head(), get_body()); }
};

/**
 * Concepts
 */

class ConceptBotImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;

    ConceptBotImpl(Index index);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptBotImpl(const ConceptBotImpl& other) = delete;
    ConceptBotImpl& operator=(const ConceptBotImpl& other) = delete;
    ConceptBotImpl(ConceptBotImpl&& other) = default;
    ConceptBotImpl& operator=(ConceptBotImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(); }
};

class ConceptTopImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;

    ConceptTopImpl(Index index);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptTopImpl(const ConceptTopImpl& other) = delete;
    ConceptTopImpl& operator=(const ConceptTopImpl& other) = delete;
    ConceptTopImpl(ConceptTopImpl&& other) = default;
    ConceptTopImpl& operator=(ConceptTopImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicStateImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptAtomicStateImpl(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl& operator=(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl(ConceptAtomicStateImpl&& other) = default;
    ConceptAtomicStateImpl& operator=(ConceptAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    formalism::Predicate<P> get_predicate() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate()); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicGoalImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;
    bool m_polarity;

    ConceptAtomicGoalImpl(Index index, formalism::Predicate<P> predicate, bool polarity);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptAtomicGoalImpl(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl& operator=(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl(ConceptAtomicGoalImpl&& other) = default;
    ConceptAtomicGoalImpl& operator=(ConceptAtomicGoalImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    formalism::Predicate<P> get_predicate() const;
    bool get_polarity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate(), get_polarity()); }
};

class ConceptIntersectionImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<ConceptTag> m_left_concept;
    NonTerminal<ConceptTag> m_right_concept;

    ConceptIntersectionImpl(Index index, NonTerminal<ConceptTag> left_concept, NonTerminal<ConceptTag> right_concept);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptIntersectionImpl(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl& operator=(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl(ConceptIntersectionImpl&& other) = default;
    ConceptIntersectionImpl& operator=(ConceptIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<ConceptTag> get_left_concept() const;
    NonTerminal<ConceptTag> get_right_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_concept(), get_right_concept()); }
};

class ConceptUnionImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<ConceptTag> m_left_concept;
    NonTerminal<ConceptTag> m_right_concept;

    ConceptUnionImpl(Index index, NonTerminal<ConceptTag> left_concept, NonTerminal<ConceptTag> right_concept);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptUnionImpl(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl& operator=(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl(ConceptUnionImpl&& other) = default;
    ConceptUnionImpl& operator=(ConceptUnionImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<ConceptTag> get_left_concept() const;
    NonTerminal<ConceptTag> get_right_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_concept(), get_right_concept()); }
};

class ConceptNegationImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<ConceptTag> m_concept;

    ConceptNegationImpl(Index index, NonTerminal<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptNegationImpl(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl& operator=(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl(ConceptNegationImpl&& other) = default;
    ConceptNegationImpl& operator=(ConceptNegationImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_concept()); }
};

class ConceptValueRestrictionImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;
    NonTerminal<ConceptTag> m_concept;

    ConceptValueRestrictionImpl(Index index, NonTerminal<RoleTag> role, NonTerminal<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptValueRestrictionImpl(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl& operator=(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl(ConceptValueRestrictionImpl&& other) = default;
    ConceptValueRestrictionImpl& operator=(ConceptValueRestrictionImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;
    NonTerminal<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role(), get_concept()); }
};

class ConceptExistentialQuantificationImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;
    NonTerminal<ConceptTag> m_concept;

    ConceptExistentialQuantificationImpl(Index index, NonTerminal<RoleTag> role, NonTerminal<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptExistentialQuantificationImpl(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl& operator=(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl(ConceptExistentialQuantificationImpl&& other) = default;
    ConceptExistentialQuantificationImpl& operator=(ConceptExistentialQuantificationImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;
    NonTerminal<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role(), get_concept()); }
};

class ConceptRoleValueMapContainmentImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_left_role;
    NonTerminal<RoleTag> m_right_role;

    ConceptRoleValueMapContainmentImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptRoleValueMapContainmentImpl(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl& operator=(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl(ConceptRoleValueMapContainmentImpl&& other) = default;
    ConceptRoleValueMapContainmentImpl& operator=(ConceptRoleValueMapContainmentImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_left_role() const;
    NonTerminal<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class ConceptRoleValueMapEqualityImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_left_role;
    NonTerminal<RoleTag> m_right_role;

    ConceptRoleValueMapEqualityImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptRoleValueMapEqualityImpl(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl& operator=(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl(ConceptRoleValueMapEqualityImpl&& other) = default;
    ConceptRoleValueMapEqualityImpl& operator=(ConceptRoleValueMapEqualityImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_left_role() const;
    NonTerminal<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class ConceptNominalImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    formalism::Object m_object;

    ConceptNominalImpl(Index index, formalism::Object object);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptNominalImpl(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl& operator=(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl(ConceptNominalImpl&& other) = default;
    ConceptNominalImpl& operator=(ConceptNominalImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    formalism::Object get_object() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_object()); }
};

/**
 * Roles
 */

class RoleUniversalImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;

    RoleUniversalImpl(Index index);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleUniversalImpl(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl& operator=(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl(RoleUniversalImpl&& other) = default;
    RoleUniversalImpl& operator=(RoleUniversalImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicStateImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleAtomicStateImpl(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl& operator=(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl(RoleAtomicStateImpl&& other) = default;
    RoleAtomicStateImpl& operator=(RoleAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    formalism::Predicate<P> get_predicate() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate()); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicGoalImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;
    bool m_polarity;

    RoleAtomicGoalImpl(Index index, formalism::Predicate<P> predicate, bool polarity);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleAtomicGoalImpl(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl& operator=(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl(RoleAtomicGoalImpl&& other) = default;
    RoleAtomicGoalImpl& operator=(RoleAtomicGoalImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    formalism::Predicate<P> get_predicate() const;
    bool get_polarity() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate(), get_polarity()); }
};

class RoleIntersectionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_left_role;
    NonTerminal<RoleTag> m_right_role;

    RoleIntersectionImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleIntersectionImpl(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl& operator=(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl(RoleIntersectionImpl&& other) = default;
    RoleIntersectionImpl& operator=(RoleIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_left_role() const;
    NonTerminal<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class RoleUnionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_left_role;
    NonTerminal<RoleTag> m_right_role;

    RoleUnionImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleUnionImpl(const RoleUnionImpl& other) = delete;
    RoleUnionImpl& operator=(const RoleUnionImpl& other) = delete;
    RoleUnionImpl(RoleUnionImpl&& other) = default;
    RoleUnionImpl& operator=(RoleUnionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_left_role() const;
    NonTerminal<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class RoleComplementImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;

    RoleComplementImpl(Index index, NonTerminal<RoleTag> role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleComplementImpl(const RoleComplementImpl& other) = delete;
    RoleComplementImpl& operator=(const RoleComplementImpl& other) = delete;
    RoleComplementImpl(RoleComplementImpl&& other) = default;
    RoleComplementImpl& operator=(RoleComplementImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleInverseImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;

    RoleInverseImpl(Index index, NonTerminal<RoleTag> role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleInverseImpl(const RoleInverseImpl& other) = delete;
    RoleInverseImpl& operator=(const RoleInverseImpl& other) = delete;
    RoleInverseImpl(RoleInverseImpl&& other) = default;
    RoleInverseImpl& operator=(RoleInverseImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleCompositionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_left_role;
    NonTerminal<RoleTag> m_right_role;

    RoleCompositionImpl(Index index, NonTerminal<RoleTag> left_role, NonTerminal<RoleTag> right_role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleCompositionImpl(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl& operator=(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl(RoleCompositionImpl&& other) = default;
    RoleCompositionImpl& operator=(RoleCompositionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_left_role() const;
    NonTerminal<RoleTag> get_right_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_role(), get_right_role()); }
};

class RoleTransitiveClosureImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;

    RoleTransitiveClosureImpl(Index index, NonTerminal<RoleTag> role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleTransitiveClosureImpl(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl& operator=(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl(RoleTransitiveClosureImpl&& other) = default;
    RoleTransitiveClosureImpl& operator=(RoleTransitiveClosureImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleReflexiveTransitiveClosureImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;

    RoleReflexiveTransitiveClosureImpl(Index index, NonTerminal<RoleTag> role);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleReflexiveTransitiveClosureImpl(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl& operator=(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl(RoleReflexiveTransitiveClosureImpl&& other) = default;
    RoleReflexiveTransitiveClosureImpl& operator=(RoleReflexiveTransitiveClosureImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role()); }
};

class RoleRestrictionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<RoleTag> m_role;
    NonTerminal<ConceptTag> m_concept;

    RoleRestrictionImpl(Index index, NonTerminal<RoleTag> role, NonTerminal<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleRestrictionImpl(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl& operator=(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl(RoleRestrictionImpl&& other) = default;
    RoleRestrictionImpl& operator=(RoleRestrictionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<RoleTag> get_role() const;
    NonTerminal<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_role(), get_concept()); }
};

class RoleIdentityImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    NonTerminal<ConceptTag> m_concept;

    RoleIdentityImpl(Index index, NonTerminal<ConceptTag> concept_);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleIdentityImpl(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl& operator=(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl(RoleIdentityImpl&& other) = default;
    RoleIdentityImpl& operator=(RoleIdentityImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<ConceptTag> get_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_concept()); }
};

/**
 * Booleans
 */

template<formalism::IsStaticOrFluentOrDerivedTag P>
class BooleanAtomicStateImpl : public IConstructor<BooleanTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    BooleanAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    BooleanAtomicStateImpl(const BooleanAtomicStateImpl& other) = delete;
    BooleanAtomicStateImpl& operator=(const BooleanAtomicStateImpl& other) = delete;
    BooleanAtomicStateImpl(BooleanAtomicStateImpl&& other) = default;
    BooleanAtomicStateImpl& operator=(BooleanAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<BooleanTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    formalism::Predicate<P> get_predicate() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_predicate()); }
};

template<IsConceptOrRoleTag D>
class BooleanNonemptyImpl : public IConstructor<BooleanTag>
{
private:
    Index m_index;
    NonTerminal<D> m_nonterminal;

    BooleanNonemptyImpl(Index index, NonTerminal<D> nonterminal);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    BooleanNonemptyImpl(const BooleanNonemptyImpl& other) = delete;
    BooleanNonemptyImpl& operator=(const BooleanNonemptyImpl& other) = delete;
    BooleanNonemptyImpl(BooleanNonemptyImpl&& other) = default;
    BooleanNonemptyImpl& operator=(BooleanNonemptyImpl&& other) = default;

    bool test_match(dl::Constructor<BooleanTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<D> get_nonterminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_nonterminal()); }
};

/**
 * Numericals
 */

template<IsConceptOrRoleTag D>
class NumericalCountImpl : public IConstructor<NumericalTag>
{
private:
    Index m_index;
    NonTerminal<D> m_nonterminal;

    NumericalCountImpl(Index index, NonTerminal<D> nonterminal);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NumericalCountImpl(const NumericalCountImpl& other) = delete;
    NumericalCountImpl& operator=(const NumericalCountImpl& other) = delete;
    NumericalCountImpl(NumericalCountImpl&& other) = default;
    NumericalCountImpl& operator=(NumericalCountImpl&& other) = default;

    bool test_match(dl::Constructor<NumericalTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<D> get_nonterminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_nonterminal()); }
};

class NumericalDistanceImpl : public IConstructor<NumericalTag>
{
private:
    Index m_index;
    NonTerminal<ConceptTag> m_left_concept;
    NonTerminal<RoleTag> m_role;
    NonTerminal<ConceptTag> m_right_concept;

    NumericalDistanceImpl(Index index, NonTerminal<ConceptTag> left_concept, NonTerminal<RoleTag> role, NonTerminal<ConceptTag> right_concept);

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NumericalDistanceImpl(const NumericalDistanceImpl& other) = delete;
    NumericalDistanceImpl& operator=(const NumericalDistanceImpl& other) = delete;
    NumericalDistanceImpl(NumericalDistanceImpl&& other) = default;
    NumericalDistanceImpl& operator=(NumericalDistanceImpl&& other) = default;

    bool test_match(dl::Constructor<NumericalTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    NonTerminal<ConceptTag> get_left_concept() const;
    NonTerminal<RoleTag> get_role() const;
    NonTerminal<ConceptTag> get_right_concept() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const { return std::tuple(get_left_concept(), get_role(), get_right_concept()); }
};
}

#endif
