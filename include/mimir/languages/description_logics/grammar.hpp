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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GRAMMAR_HPP_

#include "mimir/formalism/predicate.hpp"
#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/constructors_interface.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"

#include <functional>
#include <memory>
#include <unordered_map>
#include <vector>

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
class DerivationRule;

template<dl::IsConceptOrRole D>
class NonTerminal
{
private:
    size_t m_id;
    const DerivationRule<D>* m_rule;

public:
    NonTerminal(size_t id, const DerivationRule<D>& rule);

    bool test_match(const D& constructor) const;

    size_t get_id() const;

    const DerivationRule<D>& get_rule() const;
};

using ConceptNonTerminal = NonTerminal<dl::Concept>;
using RoleNonTerminal = NonTerminal<dl::Role>;

/**
 * Choice
 */

template<dl::IsConceptOrRole D>
using Choice = std::variant<const Constructor<D>*, const NonTerminal<D>*>;

using ConceptChoice = Choice<dl::Concept>;
using RoleChoice = Choice<dl::Role>;

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
class DerivationRule
{
protected:
    size_t m_id;
    std::vector<Choice<D>> m_choices;

public:
    DerivationRule(size_t id, std::vector<Choice<D>> choices);

    bool test_match(const D& constructor) const;

    size_t get_id() const;
};

using ConceptDerivationRule = DerivationRule<dl::Concept>;
using RoleDerivationRule = DerivationRule<dl::Role>;

/**
 * Grammar
 */

using GrammarConstructorRepositories = VariadicConstructorRepository<ConceptNonTerminal,
                                                                     ConceptDerivationRule,
                                                                     ConceptPredicateState<Static>,
                                                                     ConceptPredicateState<Fluent>,
                                                                     ConceptPredicateState<Derived>,
                                                                     ConceptPredicateGoal<Static>,
                                                                     ConceptPredicateGoal<Fluent>,
                                                                     ConceptPredicateGoal<Derived>,
                                                                     ConceptAnd,
                                                                     RoleNonTerminal,
                                                                     RoleDerivationRule,
                                                                     RolePredicateState<Static>,
                                                                     RolePredicateState<Fluent>,
                                                                     RolePredicateState<Derived>,
                                                                     RolePredicateGoal<Static>,
                                                                     RolePredicateGoal<Fluent>,
                                                                     RolePredicateGoal<Derived>,
                                                                     RoleAnd>;

class Grammar
{
private:
    /* Memory */
    GrammarConstructorRepositories m_grammar_constructor_repos;

    /* The rules of the grammar. */
    std::vector<const ConceptDerivationRule*> m_concept_rules;
    std::vector<const RoleDerivationRule*> m_role_rules;

public:
    /// @brief Create a grammar from a BNF description.
    Grammar(std::string bnf_description, Domain domain);

    /// @brief Tests whether a dl concept constructor satisfies the grammar specification.
    /// @param constructor is the dl concept constructor to test.
    /// @return true iff the dl concept constructor satisfies the grammar specification, and false otherwise.
    bool test_match(const dl::Concept& constructor) const;

    /// @brief Tests whether a dl role constructor satisfies the grammar specfication.
    /// @param constructor is the dl role constructor to test.
    /// @return true iff the dl concept constructor satisfies the grammar specification, and false otherwise.
    bool test_match(const dl::Role& constructor) const;
};
}

#endif
