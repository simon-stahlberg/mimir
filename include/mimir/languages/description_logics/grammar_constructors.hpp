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
#include "mimir/languages/description_logics/constructor_tag.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_interface.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <optional>
#include <vector>

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::ConstructorTag D>
class NonTerminalImpl
{
private:
    Index m_index;
    // Use name for equality and hash since rule is deferred instantiated.
    std::string m_name;
    // Mutable for deferred instantiation.
    mutable std::optional<DerivationRule<D>> m_rule;

    NonTerminalImpl(Index index, std::string name);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    NonTerminalImpl(const NonTerminalImpl& other) = delete;
    NonTerminalImpl& operator=(const NonTerminalImpl& other) = delete;
    NonTerminalImpl(NonTerminalImpl&& other) = default;
    NonTerminalImpl& operator=(NonTerminalImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor) const;

    void accept(Visitor& visitor) const;

    Index get_index() const;
    const std::string& get_name() const;
    DerivationRule<D> get_rule() const;

    // Deferred initialization for internal usage. Users should not use it.
    void set_rule(DerivationRule<D> rule) const;
};

/**
 * ConstructorOrNonTerminal
 */

template<dl::ConstructorTag D>
class ConstructorOrNonTerminalImpl
{
private:
    Index m_index;
    std::variant<Constructor<D>, NonTerminal<D>> m_choice;

    ConstructorOrNonTerminalImpl(Index index, std::variant<Constructor<D>, NonTerminal<D>> choice);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConstructorOrNonTerminalImpl(const ConstructorOrNonTerminalImpl& other) = delete;
    ConstructorOrNonTerminalImpl& operator=(const ConstructorOrNonTerminalImpl& other) = delete;
    ConstructorOrNonTerminalImpl(ConstructorOrNonTerminalImpl&& other) = default;
    ConstructorOrNonTerminalImpl& operator=(ConstructorOrNonTerminalImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor) const;

    void accept(Visitor& visitor) const;

    Index get_index() const;
    const std::variant<Constructor<D>, NonTerminal<D>>& get_constructor_or_non_terminal() const;
};

/**
 * DerivationRule
 */

template<dl::ConstructorTag D>
class DerivationRuleImpl
{
protected:
    Index m_index;
    NonTerminal<D> m_non_terminal;
    ConstructorOrNonTerminalList<D> m_constructor_or_non_terminals;

    DerivationRuleImpl(Index index, NonTerminal<D> non_terminal, ConstructorOrNonTerminalList<D> constructor_or_non_terminals);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    DerivationRuleImpl(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl& operator=(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl(DerivationRuleImpl&& other) = default;
    DerivationRuleImpl& operator=(DerivationRuleImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor) const;

    void accept(Visitor& visitor) const;

    Index get_index() const;
    NonTerminal<D> get_non_terminal() const;
    const ConstructorOrNonTerminalList<D>& get_constructor_or_non_terminals() const;
};

/**
 * Concepts
 */

class ConceptBotImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;

    explicit ConceptBotImpl(Index index);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptBotImpl(const ConceptBotImpl& other) = delete;
    ConceptBotImpl& operator=(const ConceptBotImpl& other) = delete;
    ConceptBotImpl(ConceptBotImpl&& other) = default;
    ConceptBotImpl& operator=(ConceptBotImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
};

class ConceptTopImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;

    explicit ConceptTopImpl(Index index);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptTopImpl(const ConceptTopImpl& other) = delete;
    ConceptTopImpl& operator=(const ConceptTopImpl& other) = delete;
    ConceptTopImpl(ConceptTopImpl&& other) = default;
    ConceptTopImpl& operator=(ConceptTopImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
};

template<PredicateTag P>
class ConceptAtomicStateImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptAtomicStateImpl(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl& operator=(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl(ConceptAtomicStateImpl&& other) = default;
    ConceptAtomicStateImpl& operator=(ConceptAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateTag P>
class ConceptAtomicGoalImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Predicate<P> m_predicate;
    bool m_is_negated;

    ConceptAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptAtomicGoalImpl(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl& operator=(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl(ConceptAtomicGoalImpl&& other) = default;
    ConceptAtomicGoalImpl& operator=(ConceptAtomicGoalImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
    bool is_negated() const;
};

class ConceptIntersectionImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal_left;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal_right;

    ConceptIntersectionImpl(Index index,
                            ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                            ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptIntersectionImpl(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl& operator=(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl(ConceptIntersectionImpl&& other) = default;
    ConceptIntersectionImpl& operator=(ConceptIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal_right() const;
};

class ConceptUnionImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal_left;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal_right;

    ConceptUnionImpl(Index index,
                     ConstructorOrNonTerminal<Concept> concept_or_non_terminal_left,
                     ConstructorOrNonTerminal<Concept> concept_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptUnionImpl(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl& operator=(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl(ConceptUnionImpl&& other) = default;
    ConceptUnionImpl& operator=(ConceptUnionImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal_right() const;
};

class ConceptNegationImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal;

    ConceptNegationImpl(Index index, ConstructorOrNonTerminal<Concept> concept_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptNegationImpl(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl& operator=(const ConceptNegationImpl& other) = delete;
    ConceptNegationImpl(ConceptNegationImpl&& other) = default;
    ConceptNegationImpl& operator=(ConceptNegationImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal() const;
};

class ConceptValueRestrictionImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal;

    ConceptValueRestrictionImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal, ConstructorOrNonTerminal<Concept> concept_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptValueRestrictionImpl(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl& operator=(const ConceptValueRestrictionImpl& other) = delete;
    ConceptValueRestrictionImpl(ConceptValueRestrictionImpl&& other) = default;
    ConceptValueRestrictionImpl& operator=(ConceptValueRestrictionImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal() const;
};

class ConceptExistentialQuantificationImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal;

    ConceptExistentialQuantificationImpl(Index index,
                                         ConstructorOrNonTerminal<Role> role_or_non_terminal,
                                         ConstructorOrNonTerminal<Concept> concept_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptExistentialQuantificationImpl(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl& operator=(const ConceptExistentialQuantificationImpl& other) = delete;
    ConceptExistentialQuantificationImpl(ConceptExistentialQuantificationImpl&& other) = default;
    ConceptExistentialQuantificationImpl& operator=(ConceptExistentialQuantificationImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal() const;
};

class ConceptRoleValueMapContainmentImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_left;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_right;

    ConceptRoleValueMapContainmentImpl(Index index,
                                       ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                       ConstructorOrNonTerminal<Role> role_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptRoleValueMapContainmentImpl(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl& operator=(const ConceptRoleValueMapContainmentImpl& other) = delete;
    ConceptRoleValueMapContainmentImpl(ConceptRoleValueMapContainmentImpl&& other) = default;
    ConceptRoleValueMapContainmentImpl& operator=(ConceptRoleValueMapContainmentImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_right() const;
};

class ConceptRoleValueMapEqualityImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_left;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_right;

    ConceptRoleValueMapEqualityImpl(Index index,
                                    ConstructorOrNonTerminal<Role> role_or_non_terminal_left,
                                    ConstructorOrNonTerminal<Role> role_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptRoleValueMapEqualityImpl(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl& operator=(const ConceptRoleValueMapEqualityImpl& other) = delete;
    ConceptRoleValueMapEqualityImpl(ConceptRoleValueMapEqualityImpl&& other) = default;
    ConceptRoleValueMapEqualityImpl& operator=(ConceptRoleValueMapEqualityImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_right() const;
};

class ConceptNominalImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Object m_object;

    ConceptNominalImpl(Index index, Object object);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ConceptNominalImpl(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl& operator=(const ConceptNominalImpl& other) = delete;
    ConceptNominalImpl(ConceptNominalImpl&& other) = default;
    ConceptNominalImpl& operator=(ConceptNominalImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    Object get_object() const;
};

/**
 * Roles
 */

class RoleUniversalImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;

    explicit RoleUniversalImpl(Index index);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleUniversalImpl(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl& operator=(const RoleUniversalImpl& other) = delete;
    RoleUniversalImpl(RoleUniversalImpl&& other) = default;
    RoleUniversalImpl& operator=(RoleUniversalImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
};

template<PredicateTag P>
class RoleAtomicStateImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleAtomicStateImpl(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl& operator=(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl(RoleAtomicStateImpl&& other) = default;
    RoleAtomicStateImpl& operator=(RoleAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateTag P>
class RoleAtomicGoalImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    Predicate<P> m_predicate;
    bool m_is_negated;

    RoleAtomicGoalImpl(Index index, Predicate<P> predicate, bool is_negated);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleAtomicGoalImpl(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl& operator=(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl(RoleAtomicGoalImpl&& other) = default;
    RoleAtomicGoalImpl& operator=(RoleAtomicGoalImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
    bool is_negated() const;
};

class RoleIntersectionImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_left;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_right;

    RoleIntersectionImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal_left, ConstructorOrNonTerminal<Role> role_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleIntersectionImpl(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl& operator=(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl(RoleIntersectionImpl&& other) = default;
    RoleIntersectionImpl& operator=(RoleIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_right() const;
};

class RoleUnionImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_left;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_right;

    RoleUnionImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal_left, ConstructorOrNonTerminal<Role> role_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleUnionImpl(const RoleUnionImpl& other) = delete;
    RoleUnionImpl& operator=(const RoleUnionImpl& other) = delete;
    RoleUnionImpl(RoleUnionImpl&& other) = default;
    RoleUnionImpl& operator=(RoleUnionImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_right() const;
};

class RoleComplementImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;

    RoleComplementImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleComplementImpl(const RoleComplementImpl& other) = delete;
    RoleComplementImpl& operator=(const RoleComplementImpl& other) = delete;
    RoleComplementImpl(RoleComplementImpl&& other) = default;
    RoleComplementImpl& operator=(RoleComplementImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
};

class RoleInverseImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;

    RoleInverseImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleInverseImpl(const RoleInverseImpl& other) = delete;
    RoleInverseImpl& operator=(const RoleInverseImpl& other) = delete;
    RoleInverseImpl(RoleInverseImpl&& other) = default;
    RoleInverseImpl& operator=(RoleInverseImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
};

class RoleCompositionImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_left;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal_right;

    RoleCompositionImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal_left, ConstructorOrNonTerminal<Role> role_or_non_terminal_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleCompositionImpl(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl& operator=(const RoleCompositionImpl& other) = delete;
    RoleCompositionImpl(RoleCompositionImpl&& other) = default;
    RoleCompositionImpl& operator=(RoleCompositionImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_left() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal_right() const;
};

class RoleTransitiveClosureImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;

    RoleTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleTransitiveClosureImpl(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl& operator=(const RoleTransitiveClosureImpl& other) = delete;
    RoleTransitiveClosureImpl(RoleTransitiveClosureImpl&& other) = default;
    RoleTransitiveClosureImpl& operator=(RoleTransitiveClosureImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
};

class RoleReflexiveTransitiveClosureImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;

    RoleReflexiveTransitiveClosureImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleReflexiveTransitiveClosureImpl(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl& operator=(const RoleReflexiveTransitiveClosureImpl& other) = delete;
    RoleReflexiveTransitiveClosureImpl(RoleReflexiveTransitiveClosureImpl&& other) = default;
    RoleReflexiveTransitiveClosureImpl& operator=(RoleReflexiveTransitiveClosureImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
};

class RoleRestrictionImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Role> m_role_or_non_terminal;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal;

    RoleRestrictionImpl(Index index, ConstructorOrNonTerminal<Role> role_or_non_terminal, ConstructorOrNonTerminal<Concept> concept_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleRestrictionImpl(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl& operator=(const RoleRestrictionImpl& other) = delete;
    RoleRestrictionImpl(RoleRestrictionImpl&& other) = default;
    RoleRestrictionImpl& operator=(RoleRestrictionImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Role> get_role_or_non_terminal() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal() const;
};

class RoleIdentityImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    ConstructorOrNonTerminal<Concept> m_concept_or_non_terminal;

    RoleIdentityImpl(Index index, ConstructorOrNonTerminal<Concept> concept_or_non_terminal);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    RoleIdentityImpl(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl& operator=(const RoleIdentityImpl& other) = delete;
    RoleIdentityImpl(RoleIdentityImpl&& other) = default;
    RoleIdentityImpl& operator=(RoleIdentityImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    void accept(Visitor& visitor) const override;

    Index get_index() const;
    ConstructorOrNonTerminal<Concept> get_concept_or_non_terminal() const;
};
}

#endif
