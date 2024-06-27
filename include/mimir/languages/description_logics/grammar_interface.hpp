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
#include <vector>

namespace mimir::dl
{

/**
 * Forward declaration
 */

template<IsDLConstructor D>
class TerminalSymbol;

template<IsDLConstructor D>
class DerivationRule;

template<PredicateCategory P>
class ConceptPredicate;
class ConceptAll;

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
 * Terminal symbols are all sorts of strings
 */

template<IsDLConstructor D>
class TerminalSymbol : public Symbol
{
public:
};

/**
 * Nonterminal symbols represent concept and role dl constructors.
 */
template<IsDLConstructor D>
class NonTerminalSymbol : public Symbol
{
protected:
    const DerivationRule<D>* m_derivation_rule;

public:
};

/**
 * Sequence rule represents recipe
 */
template<IsDLConstructor D>
class SequenceRule
{
private:
    std::vector<std::unique_ptr<Symbol>> m_sequence;

public:
    /// @brief Test whether the arguments of a concept matche the grammar specification.
    virtual bool test_match(const Concept& constructor) const { return false; }

    /// @brief Test whether the arguents of a role matche the grammar specification.
    virtual bool test_match(const Role& constructor) const { return false; }
};

template<IsDLConstructor D>
class ChoiceRule
{
private:
    std::vector<SequenceRule<D>> m_choices;

public:
    /// @brief Test whether the arguments of a concept matche the grammar specification of one of the choice rules.
    virtual bool test_match(const Concept& constructor) const { return false; }

    /// @brief Test whether the arguents of a role matche the grammar specification of one of the choice rules.
    virtual bool test_match(const Role& constructor) const { return false; }
};

template<IsDLConstructor D>
class DerivationRule
{
private:
    std::unique_ptr<NonTerminalSymbol<D>> m_head;
    std::unique_ptr<ChoiceRule<D>> m_body;

public:
    /// @brief Test whether a concept matches the grammar specification.
    virtual bool test_match(const Concept& constructor) const { return false; }

    /// @brief Test whether a role matches the grammar specification.
    virtual bool test_match(const Role& constructor) const { return false; }
};
}

#endif
