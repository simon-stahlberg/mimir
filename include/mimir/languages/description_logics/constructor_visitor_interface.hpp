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

namespace mimir::languages::dl
{

class IVisitor
{
public:
    virtual ~IVisitor() = default;

    /* Concepts */
    virtual void visit(ConceptBot constructor) = 0;
    virtual void visit(ConceptTop constructor) = 0;
    virtual void visit(ConceptAtomicState<formalism::StaticTag> constructor) = 0;
    virtual void visit(ConceptAtomicState<formalism::FluentTag> constructor) = 0;
    virtual void visit(ConceptAtomicState<formalism::DerivedTag> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<formalism::StaticTag> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<formalism::FluentTag> constructor) = 0;
    virtual void visit(ConceptAtomicGoal<formalism::DerivedTag> constructor) = 0;
    virtual void visit(ConceptIntersection constructor) = 0;
    virtual void visit(ConceptUnion constructor) = 0;
    virtual void visit(ConceptNegation constructor) = 0;
    virtual void visit(ConceptValueRestriction constructor) = 0;
    virtual void visit(ConceptExistentialQuantification constructor) = 0;
    virtual void visit(ConceptRoleValueMapContainment constructor) = 0;
    virtual void visit(ConceptRoleValueMapEquality constructor) = 0;
    virtual void visit(ConceptNominal constructor) = 0;
    /* Roles */
    virtual void visit(RoleUniversal constructor) = 0;
    virtual void visit(RoleAtomicState<formalism::StaticTag> constructor) = 0;
    virtual void visit(RoleAtomicState<formalism::FluentTag> constructor) = 0;
    virtual void visit(RoleAtomicState<formalism::DerivedTag> constructor) = 0;
    virtual void visit(RoleAtomicGoal<formalism::StaticTag> constructor) = 0;
    virtual void visit(RoleAtomicGoal<formalism::FluentTag> constructor) = 0;
    virtual void visit(RoleAtomicGoal<formalism::DerivedTag> constructor) = 0;
    virtual void visit(RoleIntersection constructor) = 0;
    virtual void visit(RoleUnion constructor) = 0;
    virtual void visit(RoleComplement constructor) = 0;
    virtual void visit(RoleInverse constructor) = 0;
    virtual void visit(RoleComposition constructor) = 0;
    virtual void visit(RoleTransitiveClosure constructor) = 0;
    virtual void visit(RoleReflexiveTransitiveClosure constructor) = 0;
    virtual void visit(RoleRestriction constructor) = 0;
    virtual void visit(RoleIdentity constructor) = 0;
    /* Booleans */
    virtual void visit(BooleanAtomicState<formalism::StaticTag> constructor) = 0;
    virtual void visit(BooleanAtomicState<formalism::FluentTag> constructor) = 0;
    virtual void visit(BooleanAtomicState<formalism::DerivedTag> constructor) = 0;
    virtual void visit(BooleanNonempty<ConceptTag> constructor) = 0;
    virtual void visit(BooleanNonempty<RoleTag> constructor) = 0;
    /* Numericals */
    virtual void visit(NumericalCount<ConceptTag> constructor) = 0;
    virtual void visit(NumericalCount<RoleTag> constructor) = 0;
    virtual void visit(NumericalDistance constructor) = 0;
};

class RecurseVisitor : public IVisitor
{
public:
    /* Concepts */
    virtual void visit(ConceptBot constructor);
    virtual void visit(ConceptTop constructor);
    virtual void visit(ConceptAtomicState<formalism::StaticTag> constructor);
    virtual void visit(ConceptAtomicState<formalism::FluentTag> constructor);
    virtual void visit(ConceptAtomicState<formalism::DerivedTag> constructor);
    virtual void visit(ConceptAtomicGoal<formalism::StaticTag> constructor);
    virtual void visit(ConceptAtomicGoal<formalism::FluentTag> constructor);
    virtual void visit(ConceptAtomicGoal<formalism::DerivedTag> constructor);
    virtual void visit(ConceptIntersection constructor);
    virtual void visit(ConceptUnion constructor);
    virtual void visit(ConceptNegation constructor);
    virtual void visit(ConceptValueRestriction constructor);
    virtual void visit(ConceptExistentialQuantification constructor);
    virtual void visit(ConceptRoleValueMapContainment constructor);
    virtual void visit(ConceptRoleValueMapEquality constructor);
    virtual void visit(ConceptNominal constructor);
    /* Roles */
    virtual void visit(RoleUniversal constructor);
    virtual void visit(RoleAtomicState<formalism::StaticTag> constructor);
    virtual void visit(RoleAtomicState<formalism::FluentTag> constructor);
    virtual void visit(RoleAtomicState<formalism::DerivedTag> constructor);
    virtual void visit(RoleAtomicGoal<formalism::StaticTag> constructor);
    virtual void visit(RoleAtomicGoal<formalism::FluentTag> constructor);
    virtual void visit(RoleAtomicGoal<formalism::DerivedTag> constructor);
    virtual void visit(RoleIntersection constructor);
    virtual void visit(RoleUnion constructor);
    virtual void visit(RoleComplement constructor);
    virtual void visit(RoleInverse constructor);
    virtual void visit(RoleComposition constructor);
    virtual void visit(RoleTransitiveClosure constructor);
    virtual void visit(RoleReflexiveTransitiveClosure constructor);
    virtual void visit(RoleRestriction constructor);
    virtual void visit(RoleIdentity constructor);
    /* Booleans */
    virtual void visit(BooleanAtomicState<formalism::StaticTag> constructor);
    virtual void visit(BooleanAtomicState<formalism::FluentTag> constructor);
    virtual void visit(BooleanAtomicState<formalism::DerivedTag> constructor);
    virtual void visit(BooleanNonempty<ConceptTag> constructor);
    virtual void visit(BooleanNonempty<RoleTag> constructor);
    /* Numericals */
    virtual void visit(NumericalCount<ConceptTag> constructor);
    virtual void visit(NumericalCount<RoleTag> constructor);
    virtual void visit(NumericalDistance constructor);
};

/**
 * GrammarVisitor
 */

class GrammarVisitor : public RecurseVisitor
{
protected:
    // Default implementations always return false
    bool m_result = false;

public:
    /* Concepts */
    void visit(ConceptBot constructor) override {}
    void visit(ConceptTop constructor) override {}
    void visit(ConceptAtomicState<formalism::StaticTag> constructor) override {}
    void visit(ConceptAtomicState<formalism::FluentTag> constructor) override {}
    void visit(ConceptAtomicState<formalism::DerivedTag> constructor) override {}
    void visit(ConceptAtomicGoal<formalism::StaticTag> constructor) override {}
    void visit(ConceptAtomicGoal<formalism::FluentTag> constructor) override {}
    void visit(ConceptAtomicGoal<formalism::DerivedTag> constructor) override {}
    void visit(ConceptIntersection constructor) override {}
    void visit(ConceptUnion constructor) override {}
    void visit(ConceptNegation constructor) override {}
    void visit(ConceptValueRestriction constructor) override {}
    void visit(ConceptExistentialQuantification constructor) override {}
    void visit(ConceptRoleValueMapContainment constructor) override {}
    void visit(ConceptRoleValueMapEquality constructor) override {}
    void visit(ConceptNominal constructor) override {}
    /* Roles */
    void visit(RoleUniversal constructor) override {}
    void visit(RoleAtomicState<formalism::StaticTag> constructor) override {}
    void visit(RoleAtomicState<formalism::FluentTag> constructor) override {}
    void visit(RoleAtomicState<formalism::DerivedTag> constructor) override {}
    void visit(RoleAtomicGoal<formalism::StaticTag> constructor) override {}
    void visit(RoleAtomicGoal<formalism::FluentTag> constructor) override {}
    void visit(RoleAtomicGoal<formalism::DerivedTag> constructor) override {}
    void visit(RoleIntersection constructor) override {}
    void visit(RoleUnion constructor) override {}
    void visit(RoleComplement constructor) override {}
    void visit(RoleInverse constructor) override {}
    void visit(RoleComposition constructor) override {}
    void visit(RoleTransitiveClosure constructor) override {}
    void visit(RoleReflexiveTransitiveClosure constructor) override {}
    void visit(RoleRestriction constructor) override {}
    void visit(RoleIdentity constructor) override {}
    /* Booleans */
    void visit(BooleanAtomicState<formalism::StaticTag> constructor) override {}
    void visit(BooleanAtomicState<formalism::FluentTag> constructor) override {}
    void visit(BooleanAtomicState<formalism::DerivedTag> constructor) override {}
    void visit(BooleanNonempty<ConceptTag> constructor) override {}
    void visit(BooleanNonempty<RoleTag> constructor) override {}
    /* Numericals */
    void visit(NumericalCount<ConceptTag> constructor) override {}
    void visit(NumericalCount<RoleTag> constructor) override {}
    void visit(NumericalDistance constructor) override {}

    bool get_result() const { return m_result; }
};

}

#endif
