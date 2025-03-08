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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_VISITOR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_VISITOR_INTERFACE_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

namespace mimir::dl
{

template<FeatureCategory D>
class ConstructorVisitor
{
};

template<>
class ConstructorVisitor<Concept>
{
protected:
    ConstructorVisitor<Role>* m_role_visitor;

public:
    virtual ~ConstructorVisitor() = default;

    virtual void initialize(ConstructorVisitor<Role>& role_visitor);

    /* Concepts */
    virtual void visit(ConceptBot constructor);
    virtual void visit(ConceptTop constructor);
    virtual void visit(ConceptAtomicState<Static> constructor);
    virtual void visit(ConceptAtomicState<Fluent> constructor);
    virtual void visit(ConceptAtomicState<Derived> constructor);
    virtual void visit(ConceptAtomicGoal<Static> constructor);
    virtual void visit(ConceptAtomicGoal<Fluent> constructor);
    virtual void visit(ConceptAtomicGoal<Derived> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);
    virtual void visit(ConceptNominal constructor);
};

template<>
class ConstructorVisitor<Role>
{
protected:
    ConstructorVisitor<Concept>* m_concept_visitor;

public:
    virtual void initialize(ConstructorVisitor<Concept>& concept_visitor);

    virtual ~ConstructorVisitor() = default;

    /* Roles */
    virtual void visit(RoleUniversal constructor);
    virtual void visit(RoleAtomicState<Static> constructor);
    virtual void visit(RoleAtomicState<Fluent> constructor);
    virtual void visit(RoleAtomicState<Derived> constructor);
    virtual void visit(RoleAtomicGoal<Static> constructor);
    virtual void visit(RoleAtomicGoal<Fluent> constructor);
    virtual void visit(RoleAtomicGoal<Derived> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
};

/**
 * GrammarVisitor
 */

template<FeatureCategory D>
class GrammarConstructorVisitor : public ConstructorVisitor<D>
{
};

template<>
class GrammarConstructorVisitor<Concept> : public ConstructorVisitor<Concept>
{
protected:
    // Default implementations always return false
    bool m_result = false;

public:
    /* Concepts */
    void visit(ConceptBot constructor) override {}
    void visit(ConceptTop constructor) override {}
    void visit(ConceptAtomicState<Static> constructor) override {}
    void visit(ConceptAtomicState<Fluent> constructor) override {}
    void visit(ConceptAtomicState<Derived> constructor) override {}
    void visit(ConceptAtomicGoal<Static> constructor) override {}
    void visit(ConceptAtomicGoal<Fluent> constructor) override {}
    void visit(ConceptAtomicGoal<Derived> constructor) override {}
    void visit(ConceptIntersection constructor) override {}
    void visit(ConceptUnion constructor) override {}
    void visit(ConceptNegation constructor) override {}
    void visit(ConceptValueRestriction constructor) override {}
    void visit(ConceptExistentialQuantification constructor) override {}
    void visit(ConceptRoleValueMapContainment constructor) override {}
    void visit(ConceptRoleValueMapEquality constructor) override {}
    void visit(ConceptNominal constructor) override {}

    bool get_result() const { return m_result; }
};

template<>
class GrammarConstructorVisitor<Role> : public ConstructorVisitor<Role>
{
protected:
    // Default implementations always return false
    bool m_result = false;

public:
    /* Roles */
    void visit(RoleUniversal constructor) override {}
    void visit(RoleAtomicState<Static> constructor) override {}
    void visit(RoleAtomicState<Fluent> constructor) override {}
    void visit(RoleAtomicState<Derived> constructor) override {}
    void visit(RoleAtomicGoal<Static> constructor) override {}
    void visit(RoleAtomicGoal<Fluent> constructor) override {}
    void visit(RoleAtomicGoal<Derived> constructor) override {}
    void visit(RoleIntersection constructor) override {}
    void visit(RoleUnion constructor) override {}
    void visit(RoleComplement constructor) override {}
    void visit(RoleInverse constructor) override {}
    void visit(RoleComposition constructor) override {}
    void visit(RoleTransitiveClosure constructor) override {}
    void visit(RoleReflexiveTransitiveClosure constructor) override {}
    void visit(RoleRestriction constructor) override {}
    void visit(RoleIdentity constructor) override {}

    bool get_result() const { return m_result; }
};

}

#endif
