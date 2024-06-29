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
class ConceptPredicateAnd;
}

template<PredicateCategory P>
class ConceptPredicateStateVisitor : public ConceptVisitor
{
private:
    const grammar::ConceptPredicateState<P>* m_grammar_constructor;

public:
    explicit ConceptPredicateStateVisitor(const grammar::ConceptPredicateState<P>& grammar_constructor) : m_grammar_constructor(&grammar_constructor) {}

    bool accept(const ConceptPredicateState<Static>& constructor) const override;
    bool accept(const ConceptPredicateState<Fluent>& constructor) const override;
    bool accept(const ConceptPredicateState<Derived>& constructor) const override;
};

class ConceptAndVisitor : public ConceptVisitor
{
private:
    const grammar::ConceptPredicateAnd* m_grammar_constructor;

public:
    explicit ConceptAndVisitor(const grammar::ConceptPredicateAnd& grammar_constructor) : m_grammar_constructor(&grammar_constructor) {}

    bool accept(const ConceptAnd& constructor) const override;
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
class RolePredicateAnd;
}

}

#endif
