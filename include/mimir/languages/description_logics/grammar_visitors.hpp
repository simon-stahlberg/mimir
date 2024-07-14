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
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
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
    const grammar::ConceptPredicateState<P>& m_grammar_constructor;

public:
    explicit ConceptPredicateStateVisitor(const grammar::ConceptPredicateState<P>& grammar_constructor);

    bool visit(const dl::ConceptPredicateState<P>& constructor) const override;
};

template<PredicateCategory P>
class ConceptPredicateGoalVisitor : public Visitor<Concept>
{
private:
    const ConceptPredicateGoal<P>& m_grammar_constructor;

public:
    explicit ConceptPredicateGoalVisitor(const ConceptPredicateGoal<P>& grammar_constructor);

    bool visit(const dl::ConceptPredicateGoal<P>& constructor) const override;
};

class ConceptAndVisitor : public Visitor<Concept>
{
private:
    const ConceptAnd& m_grammar_constructor;

public:
    explicit ConceptAndVisitor(const ConceptAnd& grammar_constructor);

    bool visit(const dl::ConceptAnd& constructor) const override;
};

/**
 * RoleVisitors
 */

template<PredicateCategory P>
class RolePredicateStateVisitor : public Visitor<Role>
{
private:
    const RolePredicateState<P>& m_grammar_constructor;

public:
    explicit RolePredicateStateVisitor(const RolePredicateState<P>& grammar_constructor);

    bool visit(const dl::RolePredicateState<P>& constructor) const override;
};

template<PredicateCategory P>
class RolePredicateGoalVisitor : public Visitor<Role>
{
private:
    const RolePredicateGoal<P>& m_grammar_constructor;

public:
    explicit RolePredicateGoalVisitor(const RolePredicateGoal<P>& grammar_constructor);

    bool visit(const dl::RolePredicateGoal<P>& constructor) const override;
};

class RoleAndVisitor : public Visitor<Role>
{
private:
    const RoleAnd& m_grammar_constructor;

public:
    explicit RoleAndVisitor(const RoleAnd& grammar_constructor);

    bool visit(const dl::RoleAnd& constructor) const override;
};

}

#endif