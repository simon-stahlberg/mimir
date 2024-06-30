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
    bool operator==(const NonTerminal& other) const;
    size_t hash() const;

    bool test_match(const D& constructor) const;

    size_t get_id() const;

    const DerivationRule<D>& get_rule() const;
};

using ConceptNonTerminal = NonTerminal<dl::Concept>;
using RoleNonTerminal = NonTerminal<dl::Role>;

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
class Choice
{
private:
    size_t m_id;
    std::variant<std::reference_wrapper<const Constructor<D>>, std::reference_wrapper<const NonTerminal<D>>> m_choice;

    Choice(size_t id, std::variant<std::reference_wrapper<const Constructor<D>>, std::reference_wrapper<const NonTerminal<D>>> choice);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const Choice& other) const;
    size_t hash() const;

    bool test_match(const D& constructor) const;

    size_t get_id() const;
};

using ConceptChoice = Choice<dl::Concept>;
using RoleChoice = Choice<dl::Role>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
class DerivationRule
{
protected:
    size_t m_id;
    std::vector<std::reference_wrapper<const Choice<D>>> m_choices;

    DerivationRule(size_t id, std::vector<std::reference_wrapper<const Choice<D>>> choices);

    template<typename T>
    friend class dl::ConstructorRepository;

public:
    bool operator==(const DerivationRule& other) const;
    size_t hash() const;

    bool test_match(const D& constructor) const;

    size_t get_id() const;
};

using ConceptDerivationRule = DerivationRule<dl::Concept>;
using RoleDerivationRule = DerivationRule<dl::Role>;

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
    bool operator==(const ConceptPredicateState& other) const;
    bool is_equal(const Concept& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
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
    bool operator==(const ConceptPredicateGoal& other) const;
    bool is_equal(const Concept& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
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
    bool operator==(const ConceptAnd& other) const;
    bool is_equal(const Concept& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Concept& constructor) const override;

    const ConceptChoice& get_concept_left() const;
    const ConceptChoice& get_concept_right() const;

    size_t get_id() const override;
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
    bool operator==(const RolePredicateState& other) const;
    bool is_equal(const Role& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
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
    bool operator==(const RolePredicateGoal& other) const;
    bool is_equal(const Role& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
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
    bool operator==(const RoleAnd& other) const;
    bool is_equal(const Role& other) const override;
    size_t hash() const override;

    bool test_match(const dl::Role& constructor) const override;

    const RoleChoice& get_role_left() const;
    const RoleChoice& get_role_right() const;

    size_t get_id() const override;
};

}
}

#endif
