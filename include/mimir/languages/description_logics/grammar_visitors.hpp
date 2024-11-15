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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_VISITORS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "mimir/languages/description_logics/grammar_visitors_interface.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl::grammar
{
/**
 * ConceptVisitors
 */

class ConceptBotVisitor : public Visitor<Concept>
{
private:
    ConceptBot m_grammar_constructor;

public:
    explicit ConceptBotVisitor(ConceptBot grammar_constructor);

    bool visit(dl::ConceptBot constructor) const override;
};

class ConceptTopVisitor : public Visitor<Concept>
{
private:
    ConceptTop m_grammar_constructor;

public:
    explicit ConceptTopVisitor(ConceptTop grammar_constructor);

    bool visit(dl::ConceptTop constructor) const override;
};

template<PredicateCategory P>
class ConceptAtomicStateVisitor : public Visitor<Concept>
{
private:
    grammar::ConceptAtomicState<P> m_grammar_constructor;

public:
    explicit ConceptAtomicStateVisitor(grammar::ConceptAtomicState<P> grammar_constructor);

    bool visit(dl::ConceptAtomicState<P> constructor) const override;
};

template<PredicateCategory P>
class ConceptAtomicGoalVisitor : public Visitor<Concept>
{
private:
    ConceptAtomicGoal<P> m_grammar_constructor;

public:
    explicit ConceptAtomicGoalVisitor(ConceptAtomicGoal<P> grammar_constructor);

    bool visit(dl::ConceptAtomicGoal<P> constructor) const override;
};

class ConceptIntersectionVisitor : public Visitor<Concept>
{
private:
    ConceptIntersection m_grammar_constructor;

public:
    explicit ConceptIntersectionVisitor(ConceptIntersection grammar_constructor);

    bool visit(dl::ConceptIntersection constructor) const override;
};

class ConceptUnionVisitor : public Visitor<Concept>
{
private:
    ConceptUnion m_grammar_constructor;

public:
    explicit ConceptUnionVisitor(ConceptUnion grammar_constructor);

    bool visit(dl::ConceptUnion constructor) const override;
};

class ConceptNegationVisitor : public Visitor<Concept>
{
private:
    ConceptNegation m_grammar_constructor;

public:
    explicit ConceptNegationVisitor(ConceptNegation grammar_constructor);

    bool visit(dl::ConceptNegation constructor) const override;
};

class ConceptValueRestrictionVisitor : public Visitor<Concept>
{
private:
    ConceptValueRestriction m_grammar_constructor;

public:
    explicit ConceptValueRestrictionVisitor(ConceptValueRestriction grammar_constructor);

    bool visit(dl::ConceptValueRestriction constructor) const override;
};

class ConceptExistentialQuantificationVisitor : public Visitor<Concept>
{
private:
    ConceptExistentialQuantification m_grammar_constructor;

public:
    explicit ConceptExistentialQuantificationVisitor(ConceptExistentialQuantification grammar_constructor);

    bool visit(dl::ConceptExistentialQuantification constructor) const override;
};

class ConceptRoleValueMapContainmentVisitor : public Visitor<Concept>
{
private:
    ConceptRoleValueMapContainment m_grammar_constructor;

public:
    explicit ConceptRoleValueMapContainmentVisitor(ConceptRoleValueMapContainment grammar_constructor);

    bool visit(dl::ConceptRoleValueMapContainment constructor) const override;
};

class ConceptRoleValueMapEqualityVisitor : public Visitor<Concept>
{
private:
    ConceptRoleValueMapEquality m_grammar_constructor;

public:
    explicit ConceptRoleValueMapEqualityVisitor(ConceptRoleValueMapEquality grammar_constructor);

    bool visit(dl::ConceptRoleValueMapEquality constructor) const override;
};

class ConceptNominalVisitor : public Visitor<Concept>
{
private:
    ConceptNominal m_grammar_constructor;

public:
    explicit ConceptNominalVisitor(ConceptNominal grammar_constructor);

    bool visit(dl::ConceptNominal constructor) const override;
};

/**
 * RoleVisitors
 */

class RoleUniversalVisitor : public Visitor<Role>
{
private:
    RoleUniversal m_grammar_constructor;

public:
    explicit RoleUniversalVisitor(RoleUniversal grammar_constructor);

    bool visit(dl::RoleUniversal constructor) const override;
};

template<PredicateCategory P>
class RoleAtomicStateVisitor : public Visitor<Role>
{
private:
    RoleAtomicState<P> m_grammar_constructor;

public:
    explicit RoleAtomicStateVisitor(RoleAtomicState<P> grammar_constructor);

    bool visit(dl::RoleAtomicState<P> constructor) const override;
};

template<PredicateCategory P>
class RoleAtomicGoalVisitor : public Visitor<Role>
{
private:
    RoleAtomicGoal<P> m_grammar_constructor;

public:
    explicit RoleAtomicGoalVisitor(RoleAtomicGoal<P> grammar_constructor);

    bool visit(dl::RoleAtomicGoal<P> constructor) const override;
};

class RoleIntersectionVisitor : public Visitor<Role>
{
private:
    RoleIntersection m_grammar_constructor;

public:
    explicit RoleIntersectionVisitor(RoleIntersection grammar_constructor);

    bool visit(dl::RoleIntersection constructor) const override;
};

class RoleUnionVisitor : public Visitor<Role>
{
private:
    RoleUnion m_grammar_constructor;

public:
    explicit RoleUnionVisitor(RoleUnion grammar_constructor);

    bool visit(dl::RoleUnion constructor) const override;
};

class RoleComplementVisitor : public Visitor<Role>
{
private:
    RoleComplement m_grammar_constructor;

public:
    explicit RoleComplementVisitor(RoleComplement grammar_constructor);

    bool visit(dl::RoleComplement constructor) const override;
};

class RoleInverseVisitor : public Visitor<Role>
{
private:
    RoleInverse m_grammar_constructor;

public:
    explicit RoleInverseVisitor(RoleInverse grammar_constructor);

    bool visit(dl::RoleInverse constructor) const override;
};

class RoleCompositionVisitor : public Visitor<Role>
{
private:
    RoleComposition m_grammar_constructor;

public:
    explicit RoleCompositionVisitor(RoleComposition grammar_constructor);

    bool visit(dl::RoleComposition constructor) const override;
};

class RoleTransitiveClosureVisitor : public Visitor<Role>
{
private:
    RoleTransitiveClosure m_grammar_constructor;

public:
    explicit RoleTransitiveClosureVisitor(RoleTransitiveClosure grammar_constructor);

    bool visit(dl::RoleTransitiveClosure constructor) const override;
};

class RoleReflexiveTransitiveClosureVisitor : public Visitor<Role>
{
private:
    RoleReflexiveTransitiveClosure m_grammar_constructor;

public:
    explicit RoleReflexiveTransitiveClosureVisitor(RoleReflexiveTransitiveClosure grammar_constructor);

    bool visit(dl::RoleReflexiveTransitiveClosure constructor) const override;
};

class RoleRestrictionVisitor : public Visitor<Role>
{
private:
    RoleRestriction m_grammar_constructor;

public:
    explicit RoleRestrictionVisitor(RoleRestriction grammar_constructor);

    bool visit(dl::RoleRestriction constructor) const override;
};

class RoleIdentityVisitor : public Visitor<Role>
{
private:
    RoleIdentity m_grammar_constructor;

public:
    explicit RoleIdentityVisitor(RoleIdentity grammar_constructor);

    bool visit(dl::RoleIdentity constructor) const override;
};

}

#endif