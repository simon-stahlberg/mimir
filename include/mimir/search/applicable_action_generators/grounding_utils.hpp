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

#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDING_UTILS_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_GROUNDING_UTILS_HPP_

#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/search/state.hpp"

namespace mimir
{

template<PredicateCategory P>
bool ground_and_test_nullary_literals(const LiteralList<P>& literals, State state, PDDLFactories& pddl_factories)
{
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_predicate()->get_arity() == 0)
        {
            if (!state.literal_holds(pddl_factories.ground_literal(literal, {})))
            {
                return false;
            }
        }
    }
    return true;
}

template<PredicateCategory P>
bool ground_and_test_nullary_literals(const LiteralList<P>& literals, const FlatBitsetBuilder<P>& grounded_atoms_ids, PDDLFactories& pddl_factories)
{
    for (const auto& literal : literals)
    {
        if (literal->get_atom()->get_predicate()->get_arity() == 0)
        {
            const auto grounded_literal = pddl_factories.ground_literal(literal, {});

            if (grounded_atoms_ids.get(grounded_literal->get_atom()->get_identifier()) == grounded_literal->is_negated())
            {
                return false;
            }
        }
    }
    return true;
}

}
#endif
