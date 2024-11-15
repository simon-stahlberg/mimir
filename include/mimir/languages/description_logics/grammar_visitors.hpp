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

/**
 * RoleVisitors
 */

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

}

#endif