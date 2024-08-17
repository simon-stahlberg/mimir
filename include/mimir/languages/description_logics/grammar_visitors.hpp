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
class ConceptPredicateStateVisitor : public Visitor<Concept>
{
private:
    grammar::ConceptPredicateState<P> m_grammar_constructor;

public:
    explicit ConceptPredicateStateVisitor(grammar::ConceptPredicateState<P> grammar_constructor);

    bool visit(dl::ConceptPredicateState<P> constructor) const override;
};

template<PredicateCategory P>
class ConceptPredicateGoalVisitor : public Visitor<Concept>
{
private:
    ConceptPredicateGoal<P> m_grammar_constructor;

public:
    explicit ConceptPredicateGoalVisitor(ConceptPredicateGoal<P> grammar_constructor);

    bool visit(dl::ConceptPredicateGoal<P> constructor) const override;
};

class ConceptAndVisitor : public Visitor<Concept>
{
private:
    ConceptAnd m_grammar_constructor;

public:
    explicit ConceptAndVisitor(ConceptAnd grammar_constructor);

    bool visit(dl::ConceptAnd constructor) const override;
};

/**
 * RoleVisitors
 */

template<PredicateCategory P>
class RolePredicateStateVisitor : public Visitor<Role>
{
private:
    RolePredicateState<P> m_grammar_constructor;

public:
    explicit RolePredicateStateVisitor(RolePredicateState<P> grammar_constructor);

    bool visit(dl::RolePredicateState<P> constructor) const override;
};

template<PredicateCategory P>
class RolePredicateGoalVisitor : public Visitor<Role>
{
private:
    RolePredicateGoal<P> m_grammar_constructor;

public:
    explicit RolePredicateGoalVisitor(RolePredicateGoal<P> grammar_constructor);

    bool visit(dl::RolePredicateGoal<P> constructor) const override;
};

class RoleAndVisitor : public Visitor<Role>
{
private:
    RoleAnd m_grammar_constructor;

public:
    explicit RoleAndVisitor(RoleAnd grammar_constructor);

    bool visit(dl::RoleAnd constructor) const override;
};

}

#endif