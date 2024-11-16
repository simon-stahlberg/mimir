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

using ConceptBotFactory = loki::UniqueFactory<ConceptBotImpl, UniqueDLHasher<const ConceptBotImpl*>, UniqueDLEqualTo<const ConceptBotImpl*>>;
template<PredicateCategory P>
using ConceptAtomicStateFactory =
    loki::UniqueFactory<ConceptAtomicStateImpl<P>, UniqueDLHasher<const ConceptAtomicStateImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using ConceptAtomicGoalFactory =
    loki::UniqueFactory<ConceptAtomicGoalImpl<P>, UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>>;
using ConceptIntersectionFactory =
    loki::UniqueFactory<ConceptIntersectionImpl, UniqueDLHasher<const ConceptIntersectionImpl*>, UniqueDLEqualTo<const ConceptIntersectionImpl*>>;

template<PredicateCategory P>
using RolePredicateStateFactory =
    loki::UniqueFactory<RoleAtomicStateImpl<P>, UniqueDLHasher<const RoleAtomicStateImpl<P>*>, UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using RolePredicateGoalFactory =
    loki::UniqueFactory<RoleAtomicGoalImpl<P>, UniqueDLHasher<const RoleAtomicGoalImpl<P>*>, UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>>;
using RoleIntersectionFactory =
    loki::UniqueFactory<RoleIntersectionImpl, UniqueDLHasher<const RoleIntersectionImpl*>, UniqueDLEqualTo<const RoleIntersectionImpl*>>;

using VariadicGrammarConstructorFactory = loki::VariadicContainer<NonTerminalFactory<Concept>,  //
                                                                  ChoiceFactory<Concept>,
                                                                  DerivationRuleFactory<Concept>,
                                                                  ConceptAtomicStateFactory<Static>,
                                                                  ConceptAtomicStateFactory<Fluent>,
                                                                  ConceptAtomicStateFactory<Derived>,
                                                                  ConceptAtomicGoalFactory<Static>,
                                                                  ConceptAtomicGoalFactory<Fluent>,
                                                                  ConceptAtomicGoalFactory<Derived>,
                                                                  ConceptIntersectionFactory,
                                                                  NonTerminalFactory<Role>,
                                                                  ChoiceFactory<Role>,
                                                                  DerivationRuleFactory<Role>,
                                                                  RolePredicateStateFactory<Static>,
                                                                  RolePredicateStateFactory<Fluent>,
                                                                  RolePredicateStateFactory<Derived>,
                                                                  RolePredicateGoalFactory<Static>,
                                                                  RolePredicateGoalFactory<Fluent>,
                                                                  RolePredicateGoalFactory<Derived>,
                                                                  RoleIntersectionFactory>;

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
