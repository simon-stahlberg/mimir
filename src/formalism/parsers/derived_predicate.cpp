/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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

#include "derived_predicate.hpp"

#include "conditions.hpp"
#include "mimir/formalism/factories.hpp"
#include "predicate.hpp"

namespace mimir
{
DerivedPredicate parse(loki::pddl::DerivedPredicate derived_predicate, PDDLFactories& factories)
{
    return factories.get_or_create_derived_predicate(parse(derived_predicate->get_predicate(), factories),
                                                     parse(derived_predicate->get_condition(), factories));
}

extern DerivedPredicateList parse(loki::pddl::DerivedPredicateList derived_predicate_list, PDDLFactories& factories)
{
    auto result_derived_predicate_list = DerivedPredicateList();
    for (const auto& derived_predicate : derived_predicate_list)
    {
        result_derived_predicate_list.push_back(parse(derived_predicate, factories));
    }
    return result_derived_predicate_list;
}
}
