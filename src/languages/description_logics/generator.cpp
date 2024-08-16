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

#include "mimir/languages/description_logics/generator.hpp"

namespace mimir::dl
{

VariadicConstructorFactory create_default_variadic_constructor_factory()
{
    return VariadicConstructorFactory(ConceptPredicateStateFactory<Static>(),
                                      ConceptPredicateStateFactory<Fluent>(),
                                      ConceptPredicateStateFactory<Derived>(),
                                      ConceptPredicateGoalFactory<Static>(),
                                      ConceptPredicateGoalFactory<Fluent>(),
                                      ConceptPredicateGoalFactory<Derived>(),
                                      ConceptAndFactory(),
                                      RolePredicateStateFactory<Static>(),
                                      RolePredicateStateFactory<Fluent>(),
                                      RolePredicateStateFactory<Derived>(),
                                      RolePredicateGoalFactory<Static>(),
                                      RolePredicateGoalFactory<Fluent>(),
                                      RolePredicateGoalFactory<Derived>(),
                                      RoleAndFactory());
}

}
