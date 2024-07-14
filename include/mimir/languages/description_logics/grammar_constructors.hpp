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
#include "mimir/languages/description_logics/constructor_ids.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
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
class NonTerminal
{
private:
    size_t m_id;
    // Use name for equality and hash since rule is deferred instantiated.
    std::string m_name;
    // Mutable for deferred instantiation.
    mutable std::optional<std::reference_wrapper<const DerivationRule<D>>> m_rule;

    NonTerminal(size_t id, std::string name);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    NonTerminal(const NonTerminal& other) = delete;
    NonTerminal& operator=(const NonTerminal& other) = delete;
    NonTerminal(NonTerminal&& other) = default;
    NonTerminal& operator=(NonTerminal&& other) = default;

    bool operator==(const NonTerminal& other) const;
    size_t hash() const;

    bool test_match(const dl::Constructor<D>& constructor) const;

    size_t get_id() const;
    const std::string& get_name() const;
    const DerivationRule<D>& get_rule() const;

    // Deferred initialization for internal usage. Users should not use it.
    void set_rule(const DerivationRule<D>& rule) const;
};

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
class Choice
{
private:
    size_t m_id;
    ConstructorOrNonTerminalChoice<D> m_choice;

    Choice(size_t id, ConstructorOrNonTerminalChoice<D> choice);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    Choice(const Choice& other) = delete;
    Choice& operator=(const Choice& other) = delete;
    Choice(Choice&& other) = default;
    Choice& operator=(Choice&& other) = default;

    bool operator==(const Choice& other) const;
    size_t hash() const;

    bool test_match(const dl::Constructor<D>& constructor) const;

    size_t get_id() const;
    const ConstructorOrNonTerminalChoice<D>& get_choice() const;
};

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
class DerivationRule
{
protected:
    size_t m_id;
    ChoiceList<D> m_choices;

    DerivationRule(size_t id, ChoiceList<D> choices);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    DerivationRule(const DerivationRule& other) = delete;
    DerivationRule& operator=(const DerivationRule& other) = delete;
    DerivationRule(DerivationRule&& other) = default;
    DerivationRule& operator=(DerivationRule&& other) = default;

    bool operator==(const DerivationRule& other) const;
    size_t hash() const;

    bool test_match(const dl::Constructor<D>& constructor) const;

    size_t get_id() const;
    const ChoiceList<D>& get_choices() const;
};

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public Constructor<Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateState(size_t id, Predicate<P> predicate);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateState(const ConceptPredicateState& other) = delete;
    ConceptPredicateState& operator=(const ConceptPredicateState& other) = delete;
    ConceptPredicateState(ConceptPredicateState&& other) = default;
    ConceptPredicateState& operator=(ConceptPredicateState&& other) = default;

    bool operator==(const ConceptPredicateState& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Concept>& constructor) const override;

    size_t get_id() const override;
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
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptPredicateGoal(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal& operator=(const ConceptPredicateGoal& other) = delete;
    ConceptPredicateGoal(ConceptPredicateGoal&& other) = default;
    ConceptPredicateGoal& operator=(ConceptPredicateGoal&& other) = default;

    bool operator==(const ConceptPredicateGoal& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Concept>& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

class ConceptAnd : public Constructor<Concept>
{
private:
    size_t m_id;
    std::reference_wrapper<const Choice<Concept>> m_concept_left;
    std::reference_wrapper<const Choice<Concept>> m_concept_right;

    ConceptAnd(size_t id, const Choice<Concept>& concept_left, const Choice<Concept>& concept_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptAnd(const ConceptAnd& other) = delete;
    ConceptAnd& operator=(const ConceptAnd& other) = delete;
    ConceptAnd(ConceptAnd&& other) = default;
    ConceptAnd& operator=(ConceptAnd&& other) = default;

    bool operator==(const ConceptAnd& other) const;
    bool is_equal(const Constructor<Concept>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Concept>& constructor) const override;

    size_t get_id() const override;
    const Choice<Concept>& get_concept_left() const;
    const Choice<Concept>& get_concept_right() const;
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
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RolePredicateState(const RolePredicateState& other) = delete;
    RolePredicateState& operator=(const RolePredicateState& other) = delete;
    RolePredicateState(RolePredicateState&& other) = default;
    RolePredicateState& operator=(RolePredicateState&& other) = default;

    bool operator==(const RolePredicateState& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Role>& constructor) const override;

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
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RolePredicateGoal(const RolePredicateGoal& other) = delete;
    RolePredicateGoal& operator=(const RolePredicateGoal& other) = delete;
    RolePredicateGoal(RolePredicateGoal&& other) = default;
    RolePredicateGoal& operator=(RolePredicateGoal&& other) = default;

    bool operator==(const RolePredicateGoal& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Role>& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

class RoleAnd : public Constructor<Role>
{
private:
    size_t m_id;
    std::reference_wrapper<const Choice<Role>> m_role_left;
    std::reference_wrapper<const Choice<Role>> m_role_right;

    RoleAnd(size_t id, const Choice<Role>& role_left, const Choice<Role>& role_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RoleAnd(const RoleAnd& other) = delete;
    RoleAnd& operator=(const RoleAnd& other) = delete;
    RoleAnd(RoleAnd&& other) = default;
    RoleAnd& operator=(RoleAnd&& other) = default;

    bool operator==(const RoleAnd& other) const;
    bool is_equal(const Constructor<Role>& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Constructor<Role>& constructor) const override;

    size_t get_id() const override;
    const Choice<Role>& get_role_left() const;
    const Choice<Role>& get_role_right() const;
};

}

#endif
