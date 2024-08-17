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

#include "mimir/formalism/declarations.hpp"
#include "mimir/languages/description_logics/equal_to.hpp"
#include "mimir/languages/description_logics/grammar_constructors.hpp"
#include "mimir/languages/description_logics/hash.hpp"

#include <loki/loki.hpp>

namespace mimir::dl::grammar
{

/**
 * Grammar
 */

template<IsConceptOrRole D>
using NonTerminalFactory = loki::UniqueFactory<NonTerminalImpl<D>, UniqueDLHasher<const NonTerminalImpl<D>*>, UniqueDLEqualTo<const NonTerminalImpl<D>*>>;
template<IsConceptOrRole D>
using ChoiceFactory = loki::UniqueFactory<ChoiceImpl<D>, UniqueDLHasher<const ChoiceImpl<D>*>, UniqueDLEqualTo<const ChoiceImpl<D>*>>;
template<IsConceptOrRole D>
using DerivationRuleFactory =
    loki::UniqueFactory<DerivationRuleImpl<D>, UniqueDLHasher<const DerivationRuleImpl<D>*>, UniqueDLEqualTo<const DerivationRuleImpl<D>*>>;
template<PredicateCategory P>
using ConceptPredicateStateFactory = loki::
    UniqueFactory<ConceptPredicateStateImpl<P>, UniqueDLHasher<const ConceptPredicateStateImpl<P>*>, UniqueDLEqualTo<const ConceptPredicateStateImpl<P>*>>;
template<PredicateCategory P>
using ConceptPredicateGoalFactory =
    loki::UniqueFactory<ConceptPredicateGoalImpl<P>, UniqueDLHasher<const ConceptPredicateGoalImpl<P>*>, UniqueDLEqualTo<const ConceptPredicateGoalImpl<P>*>>;
using ConceptAndFactory = loki::UniqueFactory<ConceptAndImpl, UniqueDLHasher<const ConceptAndImpl*>, UniqueDLEqualTo<const ConceptAndImpl*>>;
template<PredicateCategory P>
using RolePredicateStateFactory =
    loki::UniqueFactory<RolePredicateStateImpl<P>, UniqueDLHasher<const RolePredicateStateImpl<P>*>, UniqueDLEqualTo<const RolePredicateStateImpl<P>*>>;
template<PredicateCategory P>
using RolePredicateGoalFactory =
    loki::UniqueFactory<RolePredicateGoalImpl<P>, UniqueDLHasher<const RolePredicateGoalImpl<P>*>, UniqueDLEqualTo<const RolePredicateGoalImpl<P>*>>;
using RoleAndFactory = loki::UniqueFactory<RoleAndImpl, UniqueDLHasher<const RoleAndImpl*>, UniqueDLEqualTo<const RoleAndImpl*>>;

using VariadicGrammarConstructorFactory = loki::VariadicContainer<NonTerminalFactory<Concept>,  //
                                                                  ChoiceFactory<Concept>,
                                                                  DerivationRuleFactory<Concept>,
                                                                  ConceptPredicateStateFactory<Static>,
                                                                  ConceptPredicateStateFactory<Fluent>,
                                                                  ConceptPredicateStateFactory<Derived>,
                                                                  ConceptPredicateGoalFactory<Static>,
                                                                  ConceptPredicateGoalFactory<Fluent>,
                                                                  ConceptPredicateGoalFactory<Derived>,
                                                                  ConceptAndFactory,
                                                                  NonTerminalFactory<Role>,
                                                                  ChoiceFactory<Role>,
                                                                  DerivationRuleFactory<Role>,
                                                                  RolePredicateStateFactory<Static>,
                                                                  RolePredicateStateFactory<Fluent>,
                                                                  RolePredicateStateFactory<Derived>,
                                                                  RolePredicateGoalFactory<Static>,
                                                                  RolePredicateGoalFactory<Fluent>,
                                                                  RolePredicateGoalFactory<Derived>,
                                                                  RoleAndFactory>;

extern VariadicGrammarConstructorFactory create_default_variadic_grammar_constructor_factory();

class Grammar
{
private:
    /* Memory */
    VariadicGrammarConstructorFactory m_grammar_constructor_repos;

    /* The rules of the grammar. */
    DerivationRuleList<Concept> m_concept_rules;
    DerivationRuleList<Role> m_role_rules;

public:
    /// @brief Create a grammar from a BNF description.
    Grammar(std::string bnf_description, Domain domain);

    /// @brief Tests whether a dl concept constructor satisfies the grammar specification.
    /// @param constructor is the dl concept constructor to test.
    /// @return true iff the dl concept constructor satisfies the grammar specification, and false otherwise.
    bool test_match(dl::Constructor<Concept> constructor) const;

    /// @brief Tests whether a dl role constructor satisfies the grammar specfication.
    /// @param constructor is the dl role constructor to test.
    /// @return true iff the dl concept constructor satisfies the grammar specification, and false otherwise.
    bool test_match(dl::Constructor<Role> constructor) const;

    /**
     * Getters
     */
    const DerivationRuleList<Concept>& get_concept_rules() const;
    const DerivationRuleList<Role>& get_role_rules() const;
};
}

#endif
