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
using ConceptTopFactory = loki::UniqueFactory<ConceptTopImpl, UniqueDLHasher<const ConceptTopImpl*>, UniqueDLEqualTo<const ConceptTopImpl*>>;
template<PredicateCategory P>
using ConceptAtomicStateFactory =
    loki::UniqueFactory<ConceptAtomicStateImpl<P>, UniqueDLHasher<const ConceptAtomicStateImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using ConceptAtomicGoalFactory =
    loki::UniqueFactory<ConceptAtomicGoalImpl<P>, UniqueDLHasher<const ConceptAtomicGoalImpl<P>*>, UniqueDLEqualTo<const ConceptAtomicGoalImpl<P>*>>;
using ConceptIntersectionFactory =
    loki::UniqueFactory<ConceptIntersectionImpl, UniqueDLHasher<const ConceptIntersectionImpl*>, UniqueDLEqualTo<const ConceptIntersectionImpl*>>;
using ConceptUnionFactory = loki::UniqueFactory<ConceptUnionImpl, UniqueDLHasher<const ConceptUnionImpl*>, UniqueDLEqualTo<const ConceptUnionImpl*>>;
using ConceptNegationFactory =
    loki::UniqueFactory<ConceptNegationImpl, UniqueDLHasher<const ConceptNegationImpl*>, UniqueDLEqualTo<const ConceptNegationImpl*>>;
using ConceptValueRestrictionFactory =
    loki::UniqueFactory<ConceptValueRestrictionImpl, UniqueDLHasher<const ConceptValueRestrictionImpl*>, UniqueDLEqualTo<const ConceptValueRestrictionImpl*>>;
using ConceptExistentialQuantificationFactory = loki::UniqueFactory<ConceptExistentialQuantificationImpl,
                                                                    UniqueDLHasher<const ConceptExistentialQuantificationImpl*>,
                                                                    UniqueDLEqualTo<const ConceptExistentialQuantificationImpl*>>;
using ConceptRoleValueMapContainmentFactory = loki::UniqueFactory<ConceptRoleValueMapContainmentImpl,
                                                                  UniqueDLHasher<const ConceptRoleValueMapContainmentImpl*>,
                                                                  UniqueDLEqualTo<const ConceptRoleValueMapContainmentImpl*>>;
using ConceptRoleValueMapEqualityFactory = loki::UniqueFactory<ConceptRoleValueMapEqualityImpl,
                                                               UniqueDLHasher<const ConceptRoleValueMapEqualityImpl*>,
                                                               UniqueDLEqualTo<const ConceptRoleValueMapEqualityImpl*>>;
using ConceptNominalFactory = loki::UniqueFactory<ConceptNominalImpl, UniqueDLHasher<const ConceptNominalImpl*>, UniqueDLEqualTo<const ConceptNominalImpl*>>;

using RoleUniversalFactory = loki::UniqueFactory<RoleUniversalImpl, UniqueDLHasher<const RoleUniversalImpl*>, UniqueDLEqualTo<const RoleUniversalImpl*>>;
template<PredicateCategory P>
using RolePredicateStateFactory =
    loki::UniqueFactory<RoleAtomicStateImpl<P>, UniqueDLHasher<const RoleAtomicStateImpl<P>*>, UniqueDLEqualTo<const RoleAtomicStateImpl<P>*>>;
template<PredicateCategory P>
using RolePredicateGoalFactory =
    loki::UniqueFactory<RoleAtomicGoalImpl<P>, UniqueDLHasher<const RoleAtomicGoalImpl<P>*>, UniqueDLEqualTo<const RoleAtomicGoalImpl<P>*>>;
using RoleIntersectionFactory =
    loki::UniqueFactory<RoleIntersectionImpl, UniqueDLHasher<const RoleIntersectionImpl*>, UniqueDLEqualTo<const RoleIntersectionImpl*>>;
using RoleUnionFactory = loki::UniqueFactory<RoleUnionImpl, UniqueDLHasher<const RoleUnionImpl*>, UniqueDLEqualTo<const RoleUnionImpl*>>;
using RoleComplementFactory = loki::UniqueFactory<RoleComplementImpl, UniqueDLHasher<const RoleComplementImpl*>, UniqueDLEqualTo<const RoleComplementImpl*>>;
using RoleInverseFactory = loki::UniqueFactory<RoleInverseImpl, UniqueDLHasher<const RoleInverseImpl*>, UniqueDLEqualTo<const RoleInverseImpl*>>;
using RoleCompositionFactory =
    loki::UniqueFactory<RoleCompositionImpl, UniqueDLHasher<const RoleCompositionImpl*>, UniqueDLEqualTo<const RoleCompositionImpl*>>;
using RoleTransitiveClosureFactory =
    loki::UniqueFactory<RoleTransitiveClosureImpl, UniqueDLHasher<const RoleTransitiveClosureImpl*>, UniqueDLEqualTo<const RoleTransitiveClosureImpl*>>;
using RoleReflexiveTransitiveClosureFactory = loki::UniqueFactory<RoleReflexiveTransitiveClosureImpl,
                                                                  UniqueDLHasher<const RoleReflexiveTransitiveClosureImpl*>,
                                                                  UniqueDLEqualTo<const RoleReflexiveTransitiveClosureImpl*>>;
using RoleRestrictionFactory =
    loki::UniqueFactory<RoleRestrictionImpl, UniqueDLHasher<const RoleRestrictionImpl*>, UniqueDLEqualTo<const RoleRestrictionImpl*>>;
using RoleIdentityFactory = loki::UniqueFactory<RoleIdentityImpl, UniqueDLHasher<const RoleIdentityImpl*>, UniqueDLEqualTo<const RoleIdentityImpl*>>;

// TODO: use boost::hana::map
using VariadicGrammarConstructorFactory = loki::VariadicContainer<NonTerminalFactory<Concept>,  //
                                                                  ChoiceFactory<Concept>,
                                                                  DerivationRuleFactory<Concept>,
                                                                  ConceptBotFactory,
                                                                  ConceptTopFactory,
                                                                  ConceptAtomicStateFactory<Static>,
                                                                  ConceptAtomicStateFactory<Fluent>,
                                                                  ConceptAtomicStateFactory<Derived>,
                                                                  ConceptAtomicGoalFactory<Static>,
                                                                  ConceptAtomicGoalFactory<Fluent>,
                                                                  ConceptAtomicGoalFactory<Derived>,
                                                                  ConceptIntersectionFactory,
                                                                  ConceptUnionFactory,
                                                                  ConceptNegationFactory,
                                                                  ConceptValueRestrictionFactory,
                                                                  ConceptExistentialQuantificationFactory,
                                                                  ConceptRoleValueMapContainmentFactory,
                                                                  ConceptRoleValueMapEqualityFactory,
                                                                  ConceptNominalFactory,
                                                                  NonTerminalFactory<Role>,
                                                                  ChoiceFactory<Role>,
                                                                  DerivationRuleFactory<Role>,
                                                                  RoleUniversalFactory,
                                                                  RolePredicateStateFactory<Static>,
                                                                  RolePredicateStateFactory<Fluent>,
                                                                  RolePredicateStateFactory<Derived>,
                                                                  RolePredicateGoalFactory<Static>,
                                                                  RolePredicateGoalFactory<Fluent>,
                                                                  RolePredicateGoalFactory<Derived>,
                                                                  RoleIntersectionFactory,
                                                                  RoleUnionFactory,
                                                                  RoleComplementFactory,
                                                                  RoleInverseFactory,
                                                                  RoleCompositionFactory,
                                                                  RoleTransitiveClosureFactory,
                                                                  RoleReflexiveTransitiveClosureFactory,
                                                                  RoleRestrictionFactory,
                                                                  RoleIdentityFactory>;

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
