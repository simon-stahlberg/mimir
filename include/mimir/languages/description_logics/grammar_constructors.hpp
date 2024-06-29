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

namespace mimir::dl::grammar
{

/**
 * Forward declarations
 */

template<IsConcreteConceptOrRole D>
class ConstructorRepository;

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

    template<IsConcreteConceptOrRole D>
    friend class ConstructorRepository;

public:
    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<Concept>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

    ConceptPredicateGoal(size_t id, Predicate<P> predicate);

    template<IsConcreteConceptOrRole D>
    friend class ConstructorRepository;

public:
    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

class ConceptAnd : public Constructor<Concept>
{
private:
    size_t m_id;
    const Constructor<Concept>& m_concept_left;
    const Constructor<Concept>& m_concept_right;

    ConceptAnd(size_t id, const Constructor<Concept>& concept_left, const Constructor<Concept>& concept_right);

    template<IsConcreteConceptOrRole D>
    friend class ConstructorRepository;

public:
    bool test_match(const dl::Concept& constructor) const override;

    const Constructor<Concept>& get_concept_left() const;
    const Constructor<Concept>& get_concept_right() const;

    size_t get_id() const override;
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

public:
    explicit RolePredicateState(size_t id, Predicate<P> predicate);

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

template<PredicateCategory P>
class RolePredicateGoal : public Constructor<Role>
{
private:
    size_t m_id;
    Predicate<P> m_predicate;

public:
    explicit RolePredicateGoal(size_t id, Predicate<P> predicate);

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;

    size_t get_id() const override;
};

class RoleAnd : public Constructor<Role>
{
private:
    size_t m_id;
    const Constructor<Role>& m_role_left;
    const Constructor<Role>& m_role_right;

public:
    RoleAnd(size_t id, const Constructor<Role>& role_left, const Constructor<Role>& role_right);

    bool test_match(const dl::Role& constructor) const override;

    const Constructor<Role>& get_role_left() const;
    const Constructor<Role>& get_role_right() const;

    size_t get_id() const override;
};

}

#endif
