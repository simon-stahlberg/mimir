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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GENERATOR_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_GENERATOR_HPP_

#include "mimir/languages/description_logics/constructors.hpp"
#include "mimir/languages/description_logics/equal_to.hpp"
#include "mimir/languages/description_logics/grammar.hpp"
#include "mimir/languages/description_logics/hash.hpp"

#include <loki/loki.hpp>

namespace mimir::dl
{
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

using VariadicConstructorFactory = loki::VariadicContainer<ConceptPredicateStateFactory<Static>,
                                                           ConceptPredicateStateFactory<Fluent>,
                                                           ConceptPredicateStateFactory<Derived>,
                                                           ConceptPredicateGoalFactory<Static>,
                                                           ConceptPredicateGoalFactory<Fluent>,
                                                           ConceptPredicateGoalFactory<Derived>,
                                                           ConceptAndFactory,
                                                           RolePredicateStateFactory<Static>,
                                                           RolePredicateStateFactory<Fluent>,
                                                           RolePredicateStateFactory<Derived>,
                                                           RolePredicateGoalFactory<Static>,
                                                           RolePredicateGoalFactory<Fluent>,
                                                           RolePredicateGoalFactory<Derived>,
                                                           RoleAndFactory>;

extern VariadicConstructorFactory create_default_variadic_constructor_factory();

class Generator
{
private:
    /* Memory */
    VariadicConstructorFactory m_constructor_repos;

    /* Grammar specification */
    grammar::Grammar m_grammar;

public:
    Generator(grammar::Grammar grammar);

    /* TODO: refinement operators */
};
}

#endif
