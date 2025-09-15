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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_CONSTRUCTORS_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_interface.hpp"
#include "mimir/languages/description_logics/tags.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace mimir::languages::dl::grammar
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

    static auto identifying_args(const std::string& name) noexcept { return std::tuple(std::cref(name)); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(std::cref(get_name())); }
};

/**
 * ConstructorOrNonTerminal
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class ConstructorOrNonTerminalImpl
{
private:
    Index m_index;
    std::variant<Constructor<D>, NonTerminal<D>> m_choice;

    ConstructorOrNonTerminalImpl(Index index, std::variant<Constructor<D>, NonTerminal<D>> choice);

    static auto identifying_args(std::variant<Constructor<D>, NonTerminal<D>> choice) noexcept { return std::tuple(choice); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConstructorOrNonTerminalImpl(const ConstructorOrNonTerminalImpl& other) = delete;
    ConstructorOrNonTerminalImpl& operator=(const ConstructorOrNonTerminalImpl& other) = delete;
    ConstructorOrNonTerminalImpl(ConstructorOrNonTerminalImpl&& other) = default;
    ConstructorOrNonTerminalImpl& operator=(ConstructorOrNonTerminalImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor, const Grammar& grammar) const;

    void accept(IVisitor& visitor) const;

    Index get_index() const;
    const std::variant<Constructor<D>, NonTerminal<D>>& get_constructor_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_constructor_or_non_terminal()); }
};

/**
 * DerivationRule
 */

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
class DerivationRuleImpl
{
protected:
    Index m_index;
    NonTerminal<D> m_non_terminal;
    ConstructorOrNonTerminalList<D> m_constructor_or_non_terminals;

    DerivationRuleImpl(Index index, NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals);

    static auto identifying_args(NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals) noexcept
    {
        return std::tuple(non_terminal, constructor_or_non_terminals);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    using Category = D;

    // moveable but not copyable
    DerivationRuleImpl(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl& operator=(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl(DerivationRuleImpl&& other) = default;
    DerivationRuleImpl& operator=(DerivationRuleImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor, const Grammar& grammar) const;

    void accept(IVisitor& visitor) const;

    Index get_index() const;
    NonTerminal<D> get_non_terminal() const;
    const ConstructorOrNonTerminalList<D>& get_constructor_or_non_terminals() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_non_terminal(), get_constructor_or_non_terminals()); }
};

/**
 * Concepts
 */

class ConceptBotImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;

    explicit ConceptBotImpl(Index index);

    static auto identifying_args() noexcept { return std::tuple(); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(); }
};

class ConceptTopImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;

    explicit ConceptTopImpl(Index index);

    static auto identifying_args() noexcept { return std::tuple(); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicStateImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    static auto identifying_args(formalism::Predicate<P> predicate) noexcept { return std::tuple(predicate); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(get_predicate()); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class ConceptAtomicGoalImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;
    bool m_polarity;

    ConceptAtomicGoalImpl(Index index, formalism::Predicate<P> predicate, bool polarity);

    static auto identifying_args(formalism::Predicate<P> predicate, bool polarity) noexcept { return std::tuple(predicate, polarity); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(get_predicate(), get_polarity()); }
};

class ConceptIntersectionImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<ConceptTag> m_left_concept_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> m_right_concept_or_non_terminal;

    ConceptIntersectionImpl(Index index,
                            ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal,
                            ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal,
                                 ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal) noexcept
    {
        return std::tuple(left_concept_or_non_terminal, right_concept_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptIntersectionImpl(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl& operator=(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl(ConceptIntersectionImpl&& other) = default;
    ConceptIntersectionImpl& operator=(ConceptIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<ConceptTag> get_left_concept_or_non_terminal() const;
    ConstructorOrNonTerminal<ConceptTag> get_right_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_concept_or_non_terminal(), get_right_concept_or_non_terminal()); }
};

class ConceptUnionImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<ConceptTag> m_left_concept_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> m_right_concept_or_non_terminal;

    ConceptUnionImpl(Index index,
                     ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal,
                     ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<ConceptTag> left_concept_or_non_terminal,
                                 ConstructorOrNonTerminal<ConceptTag> right_concept_or_non_terminal) noexcept
    {
        return std::tuple(left_concept_or_non_terminal, right_concept_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptUnionImpl(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl& operator=(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl(ConceptUnionImpl&& other) = default;
    ConceptUnionImpl& operator=(ConceptUnionImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<ConceptTag> get_left_concept_or_non_terminal() const;
    ConstructorOrNonTerminal<ConceptTag> get_right_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_concept_or_non_terminal(), get_right_concept_or_non_terminal()); }
};

class ConceptNegationImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<ConceptTag> m_concept_or_non_terminal;

    ConceptNegationImpl(Index index, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) noexcept { return std::tuple(concept_or_non_terminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptNegationImpl(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl& operator=(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl(ConceptNegationImpl&& other) = default;
    ConceptNegationImpl& operator=(ConceptNegationImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<ConceptTag> get_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_concept_or_non_terminal()); }
};

class ConceptValueRestrictionImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> m_concept_or_non_terminal;

    ConceptValueRestrictionImpl(Index index,
                                ConstructorOrNonTerminal<RoleTag> role_or_non_terminal,
                                ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) noexcept
    {
        return std::tuple(role_or_non_terminal, concept_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptValueRestrictionImpl(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl& operator=(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl(ConceptValueRestrictionImpl&& other) = default;
    ConceptValueRestrictionImpl& operator=(ConceptValueRestrictionImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;
    ConstructorOrNonTerminal<ConceptTag> get_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal(), get_concept_or_non_terminal()); }
};

class ConceptExistentialQuantificationImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> m_concept_or_non_terminal;

    ConceptExistentialQuantificationImpl(Index index,
                                         ConstructorOrNonTerminal<RoleTag> role_or_non_terminal,
                                         ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) noexcept
    {
        return std::tuple(role_or_non_terminal, concept_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptExistentialQuantificationImpl(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl& operator=(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl(ConceptExistentialQuantificationImpl&& other) = default;
    ConceptExistentialQuantificationImpl& operator=(ConceptExistentialQuantificationImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;
    ConstructorOrNonTerminal<ConceptTag> get_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal(), get_concept_or_non_terminal()); }
};

class ConceptRoleValueMapContainmentImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> m_right_role_or_non_terminal;

    ConceptRoleValueMapContainmentImpl(Index index,
                                       ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                       ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                 ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) noexcept
    {
        return std::tuple(left_role_or_non_terminal, right_role_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptRoleValueMapContainmentImpl(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl& operator=(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl(ConceptRoleValueMapContainmentImpl&& other) = default;
    ConceptRoleValueMapContainmentImpl& operator=(ConceptRoleValueMapContainmentImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_left_role_or_non_terminal() const;
    ConstructorOrNonTerminal<RoleTag> get_right_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_role_or_non_terminal(), get_right_role_or_non_terminal()); }
};

class ConceptRoleValueMapEqualityImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> m_right_role_or_non_terminal;

    ConceptRoleValueMapEqualityImpl(Index index,
                                    ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                    ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                 ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) noexcept
    {
        return std::tuple(left_role_or_non_terminal, right_role_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    ConceptRoleValueMapEqualityImpl(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl& operator=(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl(ConceptRoleValueMapEqualityImpl&& other) = default;
    ConceptRoleValueMapEqualityImpl& operator=(ConceptRoleValueMapEqualityImpl&& other) = default;

    bool test_match(dl::Constructor<ConceptTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_left_role_or_non_terminal() const;
    ConstructorOrNonTerminal<RoleTag> get_right_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_role_or_non_terminal(), get_right_role_or_non_terminal()); }
};

class ConceptNominalImpl : public IConstructor<ConceptTag>
{
private:
    Index m_index;
    formalism::Object m_object;

    ConceptNominalImpl(Index index, formalism::Object object);

    static auto identifying_args(formalism::Object object) noexcept { return std::tuple(object); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(get_object()); }
};

/**
 * Roles
 */

class RoleUniversalImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;

    explicit RoleUniversalImpl(Index index);

    static auto identifying_args() noexcept { return std::tuple(); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicStateImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, formalism::Predicate<P> predicate);

    static auto identifying_args(formalism::Predicate<P> predicate) noexcept { return std::tuple(predicate); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(get_predicate()); }
};

template<formalism::IsStaticOrFluentOrDerivedTag P>
class RoleAtomicGoalImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    formalism::Predicate<P> m_predicate;
    bool m_polarity;

    RoleAtomicGoalImpl(Index index, formalism::Predicate<P> predicate, bool polarity);

    static auto identifying_args(formalism::Predicate<P> predicate, bool polarity) noexcept { return std::tuple(predicate, polarity); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(get_predicate(), get_polarity()); }
};

class RoleIntersectionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> m_right_role_or_non_terminal;

    RoleIntersectionImpl(Index index,
                         ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                         ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                 ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) noexcept
    {
        return std::tuple(left_role_or_non_terminal, right_role_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleIntersectionImpl(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl& operator=(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl(RoleIntersectionImpl&& other) = default;
    RoleIntersectionImpl& operator=(RoleIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_left_role_or_non_terminal() const;
    ConstructorOrNonTerminal<RoleTag> get_right_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_role_or_non_terminal(), get_right_role_or_non_terminal()); }
};

class RoleUnionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> m_right_role_or_non_terminal;

    RoleUnionImpl(Index index, ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal, ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                 ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) noexcept
    {
        return std::tuple(left_role_or_non_terminal, right_role_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleUnionImpl(const RoleUnionImpl& other) = delete;
    RoleUnionImpl& operator=(const RoleUnionImpl& other) = delete;
    RoleUnionImpl(RoleUnionImpl&& other) = default;
    RoleUnionImpl& operator=(RoleUnionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_left_role_or_non_terminal() const;
    ConstructorOrNonTerminal<RoleTag> get_right_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_role_or_non_terminal(), get_right_role_or_non_terminal()); }
};

class RoleComplementImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;

    RoleComplementImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) noexcept { return std::tuple(role_or_non_terminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleComplementImpl(const RoleComplementImpl& other) = delete;
    RoleComplementImpl& operator=(const RoleComplementImpl& other) = delete;
    RoleComplementImpl(RoleComplementImpl&& other) = default;
    RoleComplementImpl& operator=(RoleComplementImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal()); }
};

class RoleInverseImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;

    RoleInverseImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) noexcept { return std::tuple(role_or_non_terminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleInverseImpl(const RoleInverseImpl& other) = delete;
    RoleInverseImpl& operator=(const RoleInverseImpl& other) = delete;
    RoleInverseImpl(RoleInverseImpl&& other) = default;
    RoleInverseImpl& operator=(RoleInverseImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal()); }
};

class RoleCompositionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_left_role_or_non_terminal;
    ConstructorOrNonTerminal<RoleTag> m_right_role_or_non_terminal;

    RoleCompositionImpl(Index index, ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal, ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> left_role_or_non_terminal,
                                 ConstructorOrNonTerminal<RoleTag> right_role_or_non_terminal) noexcept
    {
        return std::tuple(left_role_or_non_terminal, right_role_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleCompositionImpl(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl& operator=(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl(RoleCompositionImpl&& other) = default;
    RoleCompositionImpl& operator=(RoleCompositionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_left_role_or_non_terminal() const;
    ConstructorOrNonTerminal<RoleTag> get_right_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_left_role_or_non_terminal(), get_right_role_or_non_terminal()); }
};

class RoleTransitiveClosureImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;

    RoleTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) noexcept { return std::tuple(role_or_non_terminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleTransitiveClosureImpl(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl& operator=(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl(RoleTransitiveClosureImpl&& other) = default;
    RoleTransitiveClosureImpl& operator=(RoleTransitiveClosureImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal()); }
};

class RoleReflexiveTransitiveClosureImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;

    RoleReflexiveTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal) noexcept { return std::tuple(role_or_non_terminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleReflexiveTransitiveClosureImpl(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl& operator=(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl(RoleReflexiveTransitiveClosureImpl&& other) = default;
    RoleReflexiveTransitiveClosureImpl& operator=(RoleReflexiveTransitiveClosureImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal()); }
};

class RoleRestrictionImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<RoleTag> m_role_or_non_terminal;
    ConstructorOrNonTerminal<ConceptTag> m_concept_or_non_terminal;

    RoleRestrictionImpl(Index index, ConstructorOrNonTerminal<RoleTag> role_or_non_terminal, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<RoleTag> role_or_non_terminal, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) noexcept
    {
        return std::tuple(role_or_non_terminal, concept_or_non_terminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleRestrictionImpl(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl& operator=(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl(RoleRestrictionImpl&& other) = default;
    RoleRestrictionImpl& operator=(RoleRestrictionImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_non_terminal() const;
    ConstructorOrNonTerminal<ConceptTag> get_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_role_or_non_terminal(), get_concept_or_non_terminal()); }
};

class RoleIdentityImpl : public IConstructor<RoleTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<ConceptTag> m_concept_or_non_terminal;

    RoleIdentityImpl(Index index, ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal);

    static auto identifying_args(ConstructorOrNonTerminal<ConceptTag> concept_or_non_terminal) noexcept { return std::tuple(concept_or_non_terminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    RoleIdentityImpl(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl& operator=(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl(RoleIdentityImpl&& other) = default;
    RoleIdentityImpl& operator=(RoleIdentityImpl&& other) = default;

    bool test_match(dl::Constructor<RoleTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<ConceptTag> get_concept_or_non_terminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_concept_or_non_terminal()); }
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

    static auto identifying_args(formalism::Predicate<P> predicate) noexcept { return std::tuple(predicate); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

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
    auto identifying_members() const noexcept { return std::tuple(get_predicate()); }
};

template<IsConceptOrRoleTag D>
class BooleanNonemptyImpl : public IConstructor<BooleanTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<D> m_constructor_or_nonterminal;

    BooleanNonemptyImpl(Index index, ConstructorOrNonTerminal<D> constructor_or_nonterminal);

    static auto identifying_args(ConstructorOrNonTerminal<D> constructor_or_nonterminal) noexcept { return std::tuple(constructor_or_nonterminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    BooleanNonemptyImpl(const BooleanNonemptyImpl& other) = delete;
    BooleanNonemptyImpl& operator=(const BooleanNonemptyImpl& other) = delete;
    BooleanNonemptyImpl(BooleanNonemptyImpl&& other) = default;
    BooleanNonemptyImpl& operator=(BooleanNonemptyImpl&& other) = default;

    bool test_match(dl::Constructor<BooleanTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<D> get_constructor_or_nonterminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_constructor_or_nonterminal()); }
};

/**
 * Numericals
 */

template<IsConceptOrRoleTag D>
class NumericalCountImpl : public IConstructor<NumericalTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<D> m_constructor_or_nonterminal;

    NumericalCountImpl(Index index, ConstructorOrNonTerminal<D> constructor_or_nonterminal);

    static auto identifying_args(ConstructorOrNonTerminal<D> constructor_or_nonterminal) noexcept { return std::tuple(constructor_or_nonterminal); }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    NumericalCountImpl(const NumericalCountImpl& other) = delete;
    NumericalCountImpl& operator=(const NumericalCountImpl& other) = delete;
    NumericalCountImpl(NumericalCountImpl&& other) = default;
    NumericalCountImpl& operator=(NumericalCountImpl&& other) = default;

    bool test_match(dl::Constructor<NumericalTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<D> get_constructor_or_nonterminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept { return std::tuple(get_constructor_or_nonterminal()); }
};

class NumericalDistanceImpl : public IConstructor<NumericalTag>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<ConceptTag> m_left_concept_or_nonterminal;
    ConstructorOrNonTerminal<RoleTag> m_role_or_nonterminal;
    ConstructorOrNonTerminal<ConceptTag> m_right_concept_or_nonterminal;

    NumericalDistanceImpl(Index index,
                          ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                          ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                          ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal);

    static auto identifying_args(ConstructorOrNonTerminal<ConceptTag> left_concept_or_nonterminal,
                                 ConstructorOrNonTerminal<RoleTag> role_or_nonterminal,
                                 ConstructorOrNonTerminal<ConceptTag> right_concept_or_nonterminal) noexcept
    {
        return std::tuple(left_concept_or_nonterminal, role_or_nonterminal, right_concept_or_nonterminal);
    }

    // Give access to the constructor.
    template<typename T, typename Hash, typename EqualTo>
    friend class loki::IndexedHashSet;

public:
    // moveable but not copyable
    NumericalDistanceImpl(const NumericalDistanceImpl& other) = delete;
    NumericalDistanceImpl& operator=(const NumericalDistanceImpl& other) = delete;
    NumericalDistanceImpl(NumericalDistanceImpl&& other) = default;
    NumericalDistanceImpl& operator=(NumericalDistanceImpl&& other) = default;

    bool test_match(dl::Constructor<NumericalTag> constructor, const Grammar& grammar) const override;

    void accept(IVisitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<ConceptTag> get_left_concept_or_nonterminal() const;
    ConstructorOrNonTerminal<RoleTag> get_role_or_nonterminal() const;
    ConstructorOrNonTerminal<ConceptTag> get_right_concept_or_nonterminal() const;

    /// @brief Return a tuple of const references to the members that uniquely identify an object.
    /// This enables the automatic generation of `loki::Hash` and `loki::EqualTo` specializations.
    /// @return a tuple containing const references to the members defining the object's identity.
    auto identifying_members() const noexcept
    {
        return std::tuple(get_left_concept_or_nonterminal(), get_role_or_nonterminal(), get_right_concept_or_nonterminal());
    }
};
}

#endif
