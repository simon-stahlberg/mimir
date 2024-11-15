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
#include "mimir/languages/description_logics/constructor_category.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_constructor_interface.hpp"
#include "mimir/languages/description_logics/grammar_visitors_interface.hpp"

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

template<dl::IsConceptOrRole D>
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
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    NonTerminalImpl(const NonTerminalImpl& other) = delete;
    NonTerminalImpl& operator=(const NonTerminalImpl& other) = delete;
    NonTerminalImpl(NonTerminalImpl&& other) = default;
    NonTerminalImpl& operator=(NonTerminalImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor) const;

    Index get_index() const;
    const std::string& get_name() const;
    DerivationRule<D> get_rule() const;

    // Deferred initialization for internal usage. Users should not use it.
    void set_rule(DerivationRule<D> rule) const;
};

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
class ChoiceImpl
{
private:
    Index m_index;
    ConstructorOrNonTerminalChoice<D> m_choice;

    ChoiceImpl(Index index, ConstructorOrNonTerminalChoice<D> choice);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ChoiceImpl(const ChoiceImpl& other) = delete;
    ChoiceImpl& operator=(const ChoiceImpl& other) = delete;
    ChoiceImpl(ChoiceImpl&& other) = default;
    ChoiceImpl& operator=(ChoiceImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor) const;

    Index get_index() const;
    const ConstructorOrNonTerminalChoice<D>& get_choice() const;
};

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
class DerivationRuleImpl
{
protected:
    Index m_index;
    ChoiceList<D> m_choices;

    DerivationRuleImpl(Index index, ChoiceList<D> choices);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    DerivationRuleImpl(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl& operator=(const DerivationRuleImpl& other) = delete;
    DerivationRuleImpl(DerivationRuleImpl&& other) = default;
    DerivationRuleImpl& operator=(DerivationRuleImpl&& other) = default;

    bool test_match(dl::Constructor<D> constructor) const;

    Index get_index() const;
    const ChoiceList<D>& get_choices() const;
};

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptAtomicStateImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    ConceptAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptAtomicStateImpl(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl& operator=(const ConceptAtomicStateImpl& other) = delete;
    ConceptAtomicStateImpl(ConceptAtomicStateImpl&& other) = default;
    ConceptAtomicStateImpl& operator=(ConceptAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptAtomicGoalImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    ConceptAtomicGoalImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptAtomicGoalImpl(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl& operator=(const ConceptAtomicGoalImpl& other) = delete;
    ConceptAtomicGoalImpl(ConceptAtomicGoalImpl&& other) = default;
    ConceptAtomicGoalImpl& operator=(ConceptAtomicGoalImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

class ConceptIntersectionImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Choice<Concept> m_concept_left;
    Choice<Concept> m_concept_right;

    ConceptIntersectionImpl(Index index, Choice<Concept> concept_left, Choice<Concept> concept_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptIntersectionImpl(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl& operator=(const ConceptIntersectionImpl& other) = delete;
    ConceptIntersectionImpl(ConceptIntersectionImpl&& other) = default;
    ConceptIntersectionImpl& operator=(ConceptIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    Index get_index() const;
    Choice<Concept> get_concept_left() const;
    Choice<Concept> get_concept_right() const;
};

class ConceptUnionImpl : public ConstructorImpl<Concept>
{
private:
    Index m_index;
    Choice<Concept> m_concept_left;
    Choice<Concept> m_concept_right;

    ConceptUnionImpl(Index index, Choice<Concept> concept_left, Choice<Concept> concept_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptUnionImpl(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl& operator=(const ConceptUnionImpl& other) = delete;
    ConceptUnionImpl(ConceptUnionImpl&& other) = default;
    ConceptUnionImpl& operator=(ConceptUnionImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    Index get_index() const;
    Choice<Concept> get_concept_left() const;
    Choice<Concept> get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RoleAtomicStateImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    RoleAtomicStateImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RoleAtomicStateImpl(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl& operator=(const RoleAtomicStateImpl& other) = delete;
    RoleAtomicStateImpl(RoleAtomicStateImpl&& other) = default;
    RoleAtomicStateImpl& operator=(RoleAtomicStateImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RoleAtomicGoalImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    Predicate<P> m_predicate;

    RoleAtomicGoalImpl(Index index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RoleAtomicGoalImpl(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl& operator=(const RoleAtomicGoalImpl& other) = delete;
    RoleAtomicGoalImpl(RoleAtomicGoalImpl&& other) = default;
    RoleAtomicGoalImpl& operator=(RoleAtomicGoalImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    Index get_index() const;
    Predicate<P> get_predicate() const;
};

class RoleIntersectionImpl : public ConstructorImpl<Role>
{
private:
    Index m_index;
    Choice<Role> m_role_left;
    Choice<Role> m_role_right;

    RoleIntersectionImpl(Index index, Choice<Role> role_left, Choice<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RoleIntersectionImpl(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl& operator=(const RoleIntersectionImpl& other) = delete;
    RoleIntersectionImpl(RoleIntersectionImpl&& other) = default;
    RoleIntersectionImpl& operator=(RoleIntersectionImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    Index get_index() const;
    Choice<Role> get_role_left() const;
    Choice<Role> get_role_right() const;
};

}

#endif
