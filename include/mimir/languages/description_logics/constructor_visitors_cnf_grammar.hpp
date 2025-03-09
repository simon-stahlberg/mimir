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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_VISITORS_CNF_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_VISITORS_CNF_GRAMMAR_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/constructor_visitor_interface.hpp"
#include "mimir/languages/description_logics/declarations.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl::cnf_grammar
{

/**
 * ConceptVisitors
 */

class ConceptBotGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    [[maybe_unused]] ConceptBot m_grammar_constructor;
    [[maybe_unused]] const Grammar& m_grammar;

public:
    explicit ConceptBotGrammarVisitor(ConceptBot grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptBot constructor) override;
};

class ConceptTopGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    [[maybe_unused]] ConceptTop m_grammar_constructor;
    [[maybe_unused]] const Grammar& m_grammar;

public:
    explicit ConceptTopGrammarVisitor(ConceptTop grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptTop constructor) override;
};

template<StaticOrFluentOrDerived P>
class ConceptAtomicStateGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptAtomicState<P> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptAtomicStateGrammarVisitor(ConceptAtomicState<P> grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptAtomicState<P> constructor) override;
};

template<StaticOrFluentOrDerived P>
class ConceptAtomicGoalGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptAtomicGoal<P> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptAtomicGoalGrammarVisitor(ConceptAtomicGoal<P> grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptAtomicGoal<P> constructor) override;
};

class ConceptIntersectionGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptIntersection m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptIntersectionGrammarVisitor(ConceptIntersection grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptIntersection constructor) override;
};

class ConceptUnionGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptUnion m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptUnionGrammarVisitor(ConceptUnion grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptUnion constructor) override;
};

class ConceptNegationGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptNegation m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptNegationGrammarVisitor(ConceptNegation grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptNegation constructor) override;
};

class ConceptValueRestrictionGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptValueRestriction m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptValueRestrictionGrammarVisitor(ConceptValueRestriction grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptValueRestriction constructor) override;
};

class ConceptExistentialQuantificationGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptExistentialQuantification m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptExistentialQuantificationGrammarVisitor(ConceptExistentialQuantification grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptExistentialQuantification constructor) override;
};

class ConceptRoleValueMapContainmentGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptRoleValueMapContainment m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptRoleValueMapContainmentGrammarVisitor(ConceptRoleValueMapContainment grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptRoleValueMapContainment constructor) override;
};

class ConceptRoleValueMapEqualityGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    ConceptRoleValueMapEquality m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit ConceptRoleValueMapEqualityGrammarVisitor(ConceptRoleValueMapEquality grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptRoleValueMapEquality constructor) override;
};

class ConceptNominalGrammarVisitor : public GrammarConstructorVisitor<Concept>
{
private:
    [[maybe_unused]] ConceptNominal m_grammar_constructor;
    [[maybe_unused]] const Grammar& m_grammar;

public:
    explicit ConceptNominalGrammarVisitor(ConceptNominal grammar_constructor, const Grammar& grammar);

    void visit(dl::ConceptNominal constructor) override;
};

/**
 * RoleVisitors
 */

class RoleUniversalGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    [[maybe_unused]] RoleUniversal m_grammar_constructor;
    [[maybe_unused]] const Grammar& m_grammar;

public:
    explicit RoleUniversalGrammarVisitor(RoleUniversal grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleUniversal constructor) override;
};

template<StaticOrFluentOrDerived P>
class RoleAtomicStateGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleAtomicState<P> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleAtomicStateGrammarVisitor(RoleAtomicState<P> grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleAtomicState<P> constructor) override;
};

template<StaticOrFluentOrDerived P>
class RoleAtomicGoalGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleAtomicGoal<P> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleAtomicGoalGrammarVisitor(RoleAtomicGoal<P> grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleAtomicGoal<P> constructor) override;
};

class RoleIntersectionGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleIntersection m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleIntersectionGrammarVisitor(RoleIntersection grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleIntersection constructor) override;
};

class RoleUnionGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleUnion m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleUnionGrammarVisitor(RoleUnion grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleUnion constructor) override;
};

class RoleComplementGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleComplement m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleComplementGrammarVisitor(RoleComplement grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleComplement constructor) override;
};

class RoleInverseGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleInverse m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleInverseGrammarVisitor(RoleInverse grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleInverse constructor) override;
};

class RoleCompositionGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleComposition m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleCompositionGrammarVisitor(RoleComposition grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleComposition constructor) override;
};

class RoleTransitiveClosureGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleTransitiveClosure m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleTransitiveClosureGrammarVisitor(RoleTransitiveClosure grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleTransitiveClosure constructor) override;
};

class RoleReflexiveTransitiveClosureGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleReflexiveTransitiveClosure m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleReflexiveTransitiveClosureGrammarVisitor(RoleReflexiveTransitiveClosure grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleReflexiveTransitiveClosure constructor) override;
};

class RoleRestrictionGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleRestriction m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleRestrictionGrammarVisitor(RoleRestriction grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleRestriction constructor) override;
};

class RoleIdentityGrammarVisitor : public GrammarConstructorVisitor<Role>
{
private:
    RoleIdentity m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit RoleIdentityGrammarVisitor(RoleIdentity grammar_constructor, const Grammar& grammar);

    void visit(dl::RoleIdentity constructor) override;
};

/**
 * BooleanVisitors
 */

template<StaticOrFluentOrDerived P>
class BooleanAtomicStateGrammarVisitor : public GrammarConstructorVisitor<Boolean>
{
private:
    BooleanAtomicState<P> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit BooleanAtomicStateGrammarVisitor(BooleanAtomicState<P> grammar_constructor, const Grammar& grammar);

    void visit(dl::BooleanAtomicState<P> constructor) override;
};

template<DescriptionLogicCategory D>
class BooleanNonemptyGrammarVisitor : public GrammarConstructorVisitor<Boolean>
{
private:
    BooleanNonempty<D> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit BooleanNonemptyGrammarVisitor(BooleanNonempty<D> grammar_constructor, const Grammar& grammar);

    void visit(dl::BooleanNonempty<D> constructor) override;
};

/**
 * NumericalVisitors
 */

template<DescriptionLogicCategory D>
class NumericalCountGrammarVisitor : public GrammarConstructorVisitor<Numerical>
{
private:
    NumericalCount<D> m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit NumericalCountGrammarVisitor(NumericalCount<D> grammar_constructor, const Grammar& grammar);

    void visit(dl::NumericalCount<D> constructor) override;
};

class NumericalDistanceGrammarVisitor : public GrammarConstructorVisitor<Numerical>
{
private:
    NumericalDistance m_grammar_constructor;
    const Grammar& m_grammar;

public:
    explicit NumericalDistanceGrammarVisitor(NumericalDistance grammar_constructor, const Grammar& grammar);

    void visit(dl::NumericalDistance constructor) override;
};

}

#endif