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
 * Concepts
 */

template<PredicateCategory P>
class ConceptPredicateState : public Constructor<Concept>
{
private:
    Predicate<P> m_predicate;

public:
    explicit ConceptPredicateState(Predicate<P> predicate);

    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class ConceptPredicateGoal : public Constructor<Concept>
{
private:
    Predicate<P> m_predicate;

public:
    explicit ConceptPredicateGoal(Predicate<P> predicate);

    bool test_match(const dl::Concept& constructor) const override;

    Predicate<P> get_predicate() const;
};

class ConceptAnd : public Constructor<Concept>
{
private:
    const Constructor<Concept>& m_concept_left;
    const Constructor<Concept>& m_concept_right;

public:
    ConceptAnd(const Constructor<Concept>& concept_left, const Constructor<Concept>& concept_right);

    bool test_match(const dl::Concept& constructor) const override;

    const Constructor<Concept>& get_concept_left() const;
    const Constructor<Concept>& get_concept_right() const;
};

/**
 * Roles
 */

template<PredicateCategory P>
class RolePredicateState : public Constructor<Role>
{
private:
    Predicate<P> m_predicate;

public:
    explicit RolePredicateState(Predicate<P> predicate);

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;
};

template<PredicateCategory P>
class RolePredicateGoal : public Constructor<Role>
{
private:
    Predicate<P> m_predicate;

public:
    explicit RolePredicateGoal(Predicate<P> predicate);

    bool test_match(const dl::Role& constructor) const override;

    Predicate<P> get_predicate() const;
};

class RoleAnd : public Constructor<Role>
{
private:
    const Constructor<Role>& m_role_left;
    const Constructor<Role>& m_role_right;

public:
    RoleAnd(const Constructor<Role>& role_left, const Constructor<Role>& role_right);

    bool test_match(const dl::Role& constructor) const override;

    const Constructor<Role>& get_role_left() const;
    const Constructor<Role>& get_role_right() const;
};

}

#endif
