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

#ifndef MIMIR_SEARCH_CONDITION_GROUNDER_INTERFACE_HPP_
#define MIMIR_SEARCH_CONDITION_GROUNDER_INTERFACE_HPP_

// #include "mimir/formalism/declarations.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted/assignment_set.hpp"
#include "mimir/search/states.hpp"
#include "mimir/search/states/tags.hpp"

#include <vector>

namespace mimir
{

/**
 * Static interface class.
 */
template<typename T>
class IConditionGrounder
{
public:
    void get_bindings(const State state,
                      const AssignmentSet<Fluent>& fluent_assignment_set,
                      const AssignmentSet<Derived>& derived_assignment_set,
                      std::vector<ObjectList>& out_bindings)
    {
        static_cast<T*>(this)->get_bindings_impl(state, fluent_assignment_set, derived_assignment_set, out_bindings);
    }
};

template<IsStateTag S>
class ConditionGrounder : public IConditionGrounder<ConditionGrounder<S>>
{
};

}

#endif
