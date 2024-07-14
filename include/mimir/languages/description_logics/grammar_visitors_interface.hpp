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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITOR_INTERFACE_HPP_

#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::dl::grammar
{

template<IsConceptOrRole D>
class Visitor
{
};

template<>
class Visitor<Concept>
{
public:
    virtual ~Visitor() {}

    // Default implementations always return false

    /* Concepts */
    virtual bool visit(const dl::ConceptPredicateState<Static>& constructor) const { return false; }
    virtual bool visit(const dl::ConceptPredicateState<Fluent>& constructor) const { return false; }
    virtual bool visit(const dl::ConceptPredicateState<Derived>& constructor) const { return false; }
    virtual bool visit(const dl::ConceptPredicateGoal<Static>& constructor) const { return false; }
    virtual bool visit(const dl::ConceptPredicateGoal<Fluent>& constructor) const { return false; }
    virtual bool visit(const dl::ConceptPredicateGoal<Derived>& constructor) const { return false; }
    virtual bool visit(const dl::ConceptAnd& constructor) const { return false; }
};

/**
 * RoleVisitor
 */

template<>
class Visitor<Role>
{
public:
    virtual ~Visitor() {}

    // Default implementations always return false

    /* Roles */
    virtual bool visit(const dl::RolePredicateState<Static>& constructor) const { return false; }
    virtual bool visit(const dl::RolePredicateState<Fluent>& constructor) const { return false; }
    virtual bool visit(const dl::RolePredicateState<Derived>& constructor) const { return false; }
    virtual bool visit(const dl::RolePredicateGoal<Static>& constructor) const { return false; }
    virtual bool visit(const dl::RolePredicateGoal<Fluent>& constructor) const { return false; }
    virtual bool visit(const dl::RolePredicateGoal<Derived>& constructor) const { return false; }
    virtual bool visit(const dl::RoleAnd& constructor) const { return false; }
};

}

#endif
