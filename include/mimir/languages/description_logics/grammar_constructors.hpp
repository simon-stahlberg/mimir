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
    size_t m_index;
    // Use name for equality and hash since rule is deferred instantiated.
    std::string m_name;
    // Mutable for deferred instantiation.
    mutable std::optional<DerivationRule<D>> m_rule;

    NonTerminalImpl(size_t index, std::string name);

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

    size_t get_index() const;
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
    size_t m_index;
    ConstructorOrNonTerminalChoice<D> m_choice;

    ChoiceImpl(size_t index, ConstructorOrNonTerminalChoice<D> choice);

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

    size_t get_index() const;
    const ConstructorOrNonTerminalChoice<D>& get_choice() const;
};

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
class DerivationRuleImpl
{
protected:
    size_t m_index;
    ChoiceList<D> m_choices;

    DerivationRuleImpl(size_t index, ChoiceList<D> choices);

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

    size_t get_index() const;
    const ChoiceList<D>& get_choices() const;
};

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateStateImpl : public ConstructorImpl<Concept>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    ConceptPredicateStateImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptPredicateStateImpl(const ConceptPredicateStateImpl& other) = delete;
    ConceptPredicateStateImpl& operator=(const ConceptPredicateStateImpl& other) = delete;
    ConceptPredicateStateImpl(ConceptPredicateStateImpl&& other) = default;
    ConceptPredicateStateImpl& operator=(ConceptPredicateStateImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoalImpl : public ConstructorImpl<Concept>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    ConceptPredicateGoalImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptPredicateGoalImpl(const ConceptPredicateGoalImpl& other) = delete;
    ConceptPredicateGoalImpl& operator=(const ConceptPredicateGoalImpl& other) = delete;
    ConceptPredicateGoalImpl(ConceptPredicateGoalImpl&& other) = default;
    ConceptPredicateGoalImpl& operator=(ConceptPredicateGoalImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

class ConceptAndImpl : public ConstructorImpl<Concept>
{
private:
    size_t m_index;
    Choice<Concept> m_concept_left;
    Choice<Concept> m_concept_right;

    ConceptAndImpl(size_t index, Choice<Concept> concept_left, Choice<Concept> concept_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    ConceptAndImpl(const ConceptAndImpl& other) = delete;
    ConceptAndImpl& operator=(const ConceptAndImpl& other) = delete;
    ConceptAndImpl(ConceptAndImpl&& other) = default;
    ConceptAndImpl& operator=(ConceptAndImpl&& other) = default;

    bool test_match(dl::Constructor<Concept> constructor) const override;

    size_t get_index() const;
    Choice<Concept> get_concept_left() const;
    Choice<Concept> get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateStateImpl : public ConstructorImpl<Role>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    RolePredicateStateImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RolePredicateStateImpl(const RolePredicateStateImpl& other) = delete;
    RolePredicateStateImpl& operator=(const RolePredicateStateImpl& other) = delete;
    RolePredicateStateImpl(RolePredicateStateImpl&& other) = default;
    RolePredicateStateImpl& operator=(RolePredicateStateImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoalImpl : public ConstructorImpl<Role>
{
private:
    size_t m_index;
    Predicate<P> m_predicate;

    RolePredicateGoalImpl(size_t index, Predicate<P> predicate);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RolePredicateGoalImpl(const RolePredicateGoalImpl& other) = delete;
    RolePredicateGoalImpl& operator=(const RolePredicateGoalImpl& other) = delete;
    RolePredicateGoalImpl(RolePredicateGoalImpl&& other) = default;
    RolePredicateGoalImpl& operator=(RolePredicateGoalImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    size_t get_index() const;
    Predicate<P> get_predicate() const;
};

class RoleAndImpl : public ConstructorImpl<Role>
{
private:
    size_t m_index;
    Choice<Role> m_role_left;
    Choice<Role> m_role_right;

    RoleAndImpl(size_t index, Choice<Role> role_left, Choice<Role> role_right);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::UniqueFactory;

public:
    // moveable but not copyable
    RoleAndImpl(const RoleAndImpl& other) = delete;
    RoleAndImpl& operator=(const RoleAndImpl& other) = delete;
    RoleAndImpl(RoleAndImpl&& other) = default;
    RoleAndImpl& operator=(RoleAndImpl&& other) = default;

    bool test_match(dl::Constructor<Role> constructor) const override;

    size_t get_index() const;
    Choice<Role> get_role_left() const;
    Choice<Role> get_role_right() const;
};

}

#endif
