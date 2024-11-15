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
    virtual bool visit(dl::ConceptBot constructor) const { return false; }
    virtual bool visit(dl::ConceptTop constructor) const { return false; }
    virtual bool visit(dl::ConceptAtomicState<Static> constructor) const { return false; }
    virtual bool visit(dl::ConceptAtomicState<Fluent> constructor) const { return false; }
    virtual bool visit(dl::ConceptAtomicState<Derived> constructor) const { return false; }
    virtual bool visit(dl::ConceptAtomicGoal<Static> constructor) const { return false; }
    virtual bool visit(dl::ConceptAtomicGoal<Fluent> constructor) const { return false; }
    virtual bool visit(dl::ConceptAtomicGoal<Derived> constructor) const { return false; }
    virtual bool visit(dl::ConceptIntersection constructor) const { return false; }
    virtual bool visit(dl::ConceptUnion constructor) const { return false; }
    virtual bool visit(dl::ConceptNegation constructor) const { return false; }
    virtual bool visit(dl::ConceptValueRestriction constructor) const { return false; }
    virtual bool visit(dl::ConceptExistentialQuantification constructor) const { return false; }
    virtual bool visit(dl::ConceptRoleValueMapContainment constructor) const { return false; }
    virtual bool visit(dl::ConceptRoleValueMapEquality constructor) const { return false; }
    virtual bool visit(dl::ConceptNominal constructor) const { return false; }
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
    virtual bool visit(dl::RoleUniversal constructor) const { return false; }
    virtual bool visit(dl::RoleAtomicState<Static> constructor) const { return false; }
    virtual bool visit(dl::RoleAtomicState<Fluent> constructor) const { return false; }
    virtual bool visit(dl::RoleAtomicState<Derived> constructor) const { return false; }
    virtual bool visit(dl::RoleAtomicGoal<Static> constructor) const { return false; }
    virtual bool visit(dl::RoleAtomicGoal<Fluent> constructor) const { return false; }
    virtual bool visit(dl::RoleAtomicGoal<Derived> constructor) const { return false; }
    virtual bool visit(dl::RoleIntersection constructor) const { return false; }
    virtual bool visit(dl::RoleUnion constructor) const { return false; }
    virtual bool visit(dl::RoleComplement constructor) const { return false; }
    virtual bool visit(dl::RoleInverse constructor) const { return false; }
    virtual bool visit(dl::RoleComposition constructor) const { return false; }
    virtual bool visit(dl::RoleTransitiveClosure constructor) const { return false; }
    virtual bool visit(dl::RoleReflexiveTransitiveClosure constructor) const { return false; }
    virtual bool visit(dl::RoleRestriction constructor) const { return false; }
    virtual bool visit(dl::RoleIdentity constructor) const { return false; }
};

}

#endif
