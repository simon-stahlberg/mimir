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
 * Forward declarations
 */
class NonTerminalSymbolConceptPredicate;
class NonTerminalSymbolConceptAnd;

/**
 * ConceptVisitors
 */

class ConceptPredicateVisitor : public ConceptVisitor
{
private:
    const NonTerminalSymbolConceptPredicate* m_symbol;

public:
    explicit ConceptPredicateVisitor(const NonTerminalSymbolConceptPredicate& symbol) : m_symbol(&symbol) {}

    bool accept(const ConceptPredicate<Static>& constructor) const override;
    bool accept(const ConceptPredicate<Fluent>& constructor) const override;
    bool accept(const ConceptPredicate<Derived>& constructor) const override;
};

class ConceptAndVisitor : public ConceptVisitor
{
private:
    const NonTerminalSymbolConceptAnd* m_symbol;

public:
    explicit ConceptAndVisitor(const NonTerminalSymbolConceptAnd& symbol) : m_symbol(&symbol) {}

    bool accept(const ConceptAnd& constructor) const override;
};

/**
 * RoleVisitors
 */

}

#endif
