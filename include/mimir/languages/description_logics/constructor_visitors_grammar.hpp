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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_VISITORS_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_VISITORS_GRAMMAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/constructor_visitor_interface.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * GrammarVisitor
 */

class GrammarVisitor : public Visitor
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

/**
 * ConceptVisitors
 */

class ConceptBotGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptBot m_grammar_constructor;

public:
    explicit ConceptBotGrammarVisitor(grammar::ConceptBot grammar_constructor);

    void visit(ConceptBot constructor) override;
};

class ConceptTopGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptTop m_grammar_constructor;

public:
    explicit ConceptTopGrammarVisitor(grammar::ConceptTop grammar_constructor);

    void visit(ConceptTop constructor) override;
};

template<PredicateTag P>
class ConceptAtomicStateGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptAtomicState<P> m_grammar_constructor;

public:
    explicit ConceptAtomicStateGrammarVisitor(grammar::ConceptAtomicState<P> grammar_constructor);

    void visit(ConceptAtomicState<P> constructor) override;
};

template<PredicateTag P>
class ConceptAtomicGoalGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptAtomicGoal<P> m_grammar_constructor;

public:
    explicit ConceptAtomicGoalGrammarVisitor(grammar::ConceptAtomicGoal<P> grammar_constructor);

    void visit(ConceptAtomicGoal<P> constructor) override;
};

class ConceptIntersectionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptIntersection m_grammar_constructor;

public:
    explicit ConceptIntersectionGrammarVisitor(grammar::ConceptIntersection grammar_constructor);

    void visit(ConceptIntersection constructor) override;
};

class ConceptUnionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptUnion m_grammar_constructor;

public:
    explicit ConceptUnionGrammarVisitor(grammar::ConceptUnion grammar_constructor);

    void visit(ConceptUnion constructor) override;
};

class ConceptNegationGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptNegation m_grammar_constructor;

public:
    explicit ConceptNegationGrammarVisitor(grammar::ConceptNegation grammar_constructor);

    void visit(ConceptNegation constructor) override;
};

class ConceptValueRestrictionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptValueRestriction m_grammar_constructor;

public:
    explicit ConceptValueRestrictionGrammarVisitor(grammar::ConceptValueRestriction grammar_constructor);

    void visit(ConceptValueRestriction constructor) override;
};

class ConceptExistentialQuantificationGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptExistentialQuantification m_grammar_constructor;

public:
    explicit ConceptExistentialQuantificationGrammarVisitor(grammar::ConceptExistentialQuantification grammar_constructor);

    void visit(ConceptExistentialQuantification constructor) override;
};

class ConceptRoleValueMapContainmentGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptRoleValueMapContainment m_grammar_constructor;

public:
    explicit ConceptRoleValueMapContainmentGrammarVisitor(grammar::ConceptRoleValueMapContainment grammar_constructor);

    void visit(ConceptRoleValueMapContainment constructor) override;
};

class ConceptRoleValueMapEqualityGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptRoleValueMapEquality m_grammar_constructor;

public:
    explicit ConceptRoleValueMapEqualityGrammarVisitor(grammar::ConceptRoleValueMapEquality grammar_constructor);

    void visit(ConceptRoleValueMapEquality constructor) override;
};

class ConceptNominalGrammarVisitor : public GrammarVisitor
{
private:
    grammar::ConceptNominal m_grammar_constructor;

public:
    explicit ConceptNominalGrammarVisitor(grammar::ConceptNominal grammar_constructor);

    void visit(ConceptNominal constructor) override;
};

/**
 * RoleVisitors
 */

class RoleUniversalGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleUniversal m_grammar_constructor;

public:
    explicit RoleUniversalGrammarVisitor(grammar::RoleUniversal grammar_constructor);

    void visit(RoleUniversal constructor) override;
};

template<PredicateTag P>
class RoleAtomicStateGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleAtomicState<P> m_grammar_constructor;

public:
    explicit RoleAtomicStateGrammarVisitor(grammar::RoleAtomicState<P> grammar_constructor);

    void visit(RoleAtomicState<P> constructor) override;
};

template<PredicateTag P>
class RoleAtomicGoalGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleAtomicGoal<P> m_grammar_constructor;

public:
    explicit RoleAtomicGoalGrammarVisitor(grammar::RoleAtomicGoal<P> grammar_constructor);

    void visit(RoleAtomicGoal<P> constructor) override;
};

class RoleIntersectionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleIntersection m_grammar_constructor;

public:
    explicit RoleIntersectionGrammarVisitor(grammar::RoleIntersection grammar_constructor);

    void visit(RoleIntersection constructor) override;
};

class RoleUnionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleUnion m_grammar_constructor;

public:
    explicit RoleUnionGrammarVisitor(grammar::RoleUnion grammar_constructor);

    void visit(RoleUnion constructor) override;
};

class RoleComplementGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleComplement m_grammar_constructor;

public:
    explicit RoleComplementGrammarVisitor(grammar::RoleComplement grammar_constructor);

    void visit(RoleComplement constructor) override;
};

class RoleInverseGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleInverse m_grammar_constructor;

public:
    explicit RoleInverseGrammarVisitor(grammar::RoleInverse grammar_constructor);

    void visit(RoleInverse constructor) override;
};

class RoleCompositionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleComposition m_grammar_constructor;

public:
    explicit RoleCompositionGrammarVisitor(grammar::RoleComposition grammar_constructor);

    void visit(RoleComposition constructor) override;
};

class RoleTransitiveClosureGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleTransitiveClosure m_grammar_constructor;

public:
    explicit RoleTransitiveClosureGrammarVisitor(grammar::RoleTransitiveClosure grammar_constructor);

    void visit(RoleTransitiveClosure constructor) override;
};

class RoleReflexiveTransitiveClosureGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleReflexiveTransitiveClosure m_grammar_constructor;

public:
    explicit RoleReflexiveTransitiveClosureGrammarVisitor(grammar::RoleReflexiveTransitiveClosure grammar_constructor);

    void visit(RoleReflexiveTransitiveClosure constructor) override;
};

class RoleRestrictionGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleRestriction m_grammar_constructor;

public:
    explicit RoleRestrictionGrammarVisitor(grammar::RoleRestriction grammar_constructor);

    void visit(RoleRestriction constructor) override;
};

class RoleIdentityGrammarVisitor : public GrammarVisitor
{
private:
    grammar::RoleIdentity m_grammar_constructor;

public:
    explicit RoleIdentityGrammarVisitor(grammar::RoleIdentity grammar_constructor);

    void visit(RoleIdentity constructor) override;
};

}

#endif