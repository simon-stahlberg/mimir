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

#include "mimir/languages/description_logics/constructor_repositories.hpp"
#include "mimir/languages/description_logics/grammar.hpp"

namespace mimir::dl
{
using ConstructorRepositories = VariadicConstructorRepository<ConceptPredicateState<Static>,
                                                              ConceptPredicateState<Fluent>,
                                                              ConceptPredicateState<Derived>,
                                                              ConceptPredicateGoal<Static>,
                                                              ConceptPredicateGoal<Fluent>,
                                                              ConceptPredicateGoal<Derived>,
                                                              ConceptAnd,
                                                              RolePredicateState<Static>,
                                                              RolePredicateState<Fluent>,
                                                              RolePredicateState<Derived>,
                                                              RolePredicateGoal<Static>,
                                                              RolePredicateGoal<Fluent>,
                                                              RolePredicateGoal<Derived>,
                                                              RoleAnd>;

class Generator
{
private:
    /* Memory */
    ConstructorRepositories m_constructor_repos;

    /* Grammar specification */
    grammar::Grammar m_grammar;

public:
    Generator(grammar::Grammar grammar);

    /* TODO: refinement operators */
};
}

#endif
