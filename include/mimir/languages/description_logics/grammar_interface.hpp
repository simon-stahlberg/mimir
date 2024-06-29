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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_INTERFACE_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_INTERFACE_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

namespace mimir::dl
{

/**
 * Forward declaration
 */

class DerivationRule;

/**
 * Symbol
 */

class Symbol
{
public:
    virtual ~Symbol() {}

    /// @brief Test whether a concept matches the grammar specification.
    virtual bool test_match(const Concept& constructor) const { return false; }

    /// @brief Test whether a role matches the grammar specification.
    virtual bool test_match(const Role& constructor) const { return false; }
};

/**
 * Terminal symbols represent dl constructors
 */

class TerminalSymbol : public Symbol
{
public:
};

/**
 * Nonterminal symbols
 */

class NonTerminalSymbol : public Symbol
{
protected:
    const DerivationRule* m_derivation_rule;

public:
};

class Choice
{
protected:
    std::variant<const TerminalSymbol*, const NonTerminalSymbol*> m_choice;

public:
    /// @brief Test whether the arguments of a concept matche the grammar specification of one of the choice rules.
    virtual bool test_match(const Concept& constructor) const { return false; }

    /// @brief Test whether the arguents of a role matche the grammar specification of one of the choice rules.
    virtual bool test_match(const Role& constructor) const { return false; }
};

class DerivationRule
{
private:
    const NonTerminalSymbol* m_non_terminal;
    std::vector<const Choice*> m_choices;

public:
    /// @brief Test whether a concept matches the grammar specification.
    virtual bool test_match(const Concept& constructor) const { return false; }

    /// @brief Test whether a role matches the grammar specification.
    virtual bool test_match(const Role& constructor) const { return false; }
};
}

#endif
