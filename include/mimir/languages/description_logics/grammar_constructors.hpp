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
#include "mimir/languages/description_logics/constructors_interface.hpp"
#include "mimir/languages/description_logics/grammar_constructors_interface.hpp"

namespace mimir::dl
{
template<typename T>
class ConstructorRepository;

namespace grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
class DerivationRule;

template<dl::IsConceptOrRole D>
class NonTerminal
{
private:
    size_t m_id;
    // Use name for equality and hash since rule is deferred instantiated.
    std::string m_name;
    // Use double pointer for deferred instantiation.
    std::unique_ptr<const DerivationRule<D>*> m_rule;

    NonTerminal(size_t id, std::string name, std::unique_ptr<const DerivationRule<D>*>&& rule);

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

    bool test_match(const D& constructor) const;

    size_t get_id() const;
    const std::string& get_name() const;
    const DerivationRule<D>& get_rule() const;
};

using ConceptNonTerminal = NonTerminal<dl::Concept>;
using RoleNonTerminal = NonTerminal<dl::Role>;

/**
 * Choice
 */

template<IsConceptOrRole D>
using ConstructorOrNonTerminalChoice = std::variant<std::reference_wrapper<const Constructor<D>>, std::reference_wrapper<const NonTerminal<D>>>;

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

    bool test_match(const D& constructor) const;

    size_t get_id() const;
    const ConstructorOrNonTerminalChoice<D>& get_choice() const;
};

template<dl::IsConceptOrRole D>
using ChoiceList = std::vector<std::reference_wrapper<const Choice<D>>>;

using ConceptChoice = Choice<dl::Concept>;
using ConceptChoiceList = ChoiceList<dl::Concept>;
using RoleChoice = Choice<dl::Role>;
using RoleChoiceList = ChoiceList<dl::Role>;

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

    bool test_match(const D& constructor) const;

    size_t get_id() const;
    const ChoiceList<D>& get_choices() const;
};

template<IsConceptOrRole D>
using DerivationRuleList = std::vector<std::reference_wrapper<const DerivationRule<D>>>;

using ConceptDerivationRule = DerivationRule<dl::Concept>;
using ConceptDerivationRuleList = DerivationRuleList<dl::Concept>;
using RoleDerivationRule = DerivationRule<dl::Role>;
using RoleDerivationRuleList = DerivationRuleList<dl::Role>;

/**
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public Constructor<dl::Concept>
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
    bool is_equal(const Concept& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Concept& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<dl::Concept>
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
    bool is_equal(const Concept& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Concept& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

class ConceptAnd : public Constructor<dl::Concept>
{
private:
    size_t m_id;
    const ConceptChoice& m_concept_left;
    const ConceptChoice& m_concept_right;

    ConceptAnd(size_t id, const ConceptChoice& concept_left, const ConceptChoice& concept_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    ConceptAnd(const ConceptAnd& other) = delete;
    ConceptAnd& operator=(const ConceptAnd& other) = delete;
    ConceptAnd(ConceptAnd&& other) = default;
    ConceptAnd& operator=(ConceptAnd&& other) = default;

    bool operator==(const ConceptAnd& other) const;
    bool is_equal(const Concept& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Concept& constructor) const override;

    size_t get_id() const override;
    const ConceptChoice& get_concept_left() const;
    const ConceptChoice& get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateState : public Constructor<dl::Role>
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
    bool is_equal(const Role& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Role& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoal : public Constructor<dl::Role>
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
    bool is_equal(const Role& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Role& constructor) const override;

    size_t get_id() const override;
    Predicate<P> get_predicate() const;
};

class RoleAnd : public Constructor<dl::Role>
{
private:
    size_t m_id;
    const RoleChoice& m_role_left;
    const RoleChoice& m_role_right;

    RoleAnd(size_t id, const RoleChoice& role_left, const RoleChoice& role_right);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    // Users are not supposed to move these directly.
    RoleAnd(const RoleAnd& other) = delete;
    RoleAnd& operator=(const RoleAnd& other) = delete;
    RoleAnd(RoleAnd&& other) = default;
    RoleAnd& operator=(RoleAnd&& other) = default;

    bool operator==(const RoleAnd& other) const;
    bool is_equal(const Role& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Role& constructor) const override;

    size_t get_id() const override;
    const RoleChoice& get_role_left() const;
    const RoleChoice& get_role_right() const;
};

}
}

#endif
