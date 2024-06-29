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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_VISITORS_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_VISITORS_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/visitors_interface.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <vector>

namespace mimir::dl
{

/**
 * ConceptVisitors
 */

namespace grammar
{
template<PredicateCategory P>
class ConceptPredicateState;
template<PredicateCategory P>
class ConceptPredicateGoal;
class ConceptAnd;
}

template<PredicateCategory P>
class ConceptPredicateStateVisitor : public ConceptVisitor
{
private:
    const grammar::ConceptPredicateState<P>& m_grammar_constructor;

public:
    explicit ConceptPredicateStateVisitor(const grammar::ConceptPredicateState<P>& grammar_constructor);

    bool visit(const ConceptPredicateState<P>& constructor) const override;
};

template<PredicateCategory P>
class ConceptPredicateGoalVisitor : public ConceptVisitor
{
private:
    const grammar::ConceptPredicateGoal<P>& m_grammar_constructor;

public:
    explicit ConceptPredicateGoalVisitor(const grammar::ConceptPredicateGoal<P>& grammar_constructor);

    bool visit(const ConceptPredicateGoal<P>& constructor) const override;
};

class ConceptAndVisitor : public ConceptVisitor
{
private:
    const grammar::ConceptAnd& m_grammar_constructor;

public:
    explicit ConceptAndVisitor(const grammar::ConceptAnd& grammar_constructor);

    bool visit(const ConceptAnd& constructor) const override;
};

/**
 * RoleVisitors
 */

namespace grammar
{
template<PredicateCategory P>
class RolePredicateState;
template<PredicateCategory P>
class RolePredicateGoal;
class RoleAnd;
}

template<PredicateCategory P>
class RolePredicateStateVisitor : public RoleVisitor
{
private:
    const grammar::RolePredicateState<P>& m_grammar_constructor;

public:
    explicit RolePredicateStateVisitor(const grammar::RolePredicateState<P>& grammar_constructor);

    bool visit(const RolePredicateState<P>& constructor) const override;
};

template<PredicateCategory P>
class RolePredicateGoalVisitor : public RoleVisitor
{
private:
    const grammar::RolePredicateGoal<P>& m_grammar_constructor;

public:
    explicit RolePredicateGoalVisitor(const grammar::RolePredicateGoal<P>& grammar_constructor);

    bool visit(const RolePredicateGoal<P>& constructor) const override;
};

class RoleAndVisitor : public RoleVisitor
{
private:
    const grammar::RoleAnd& m_grammar_constructor;

public:
    explicit RoleAndVisitor(const grammar::RoleAnd& grammar_constructor);

    bool visit(const RoleAnd& constructor) const override;
};

}

#endif
