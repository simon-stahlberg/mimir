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

#include <mimir/formalism/domain/parsers/function.hpp>
#include <mimir/formalism/problem/parsers/ground_atom.hpp>
#include <mimir/formalism/problem/parsers/numeric_fluent.hpp>

namespace mimir
{
NumericFluent parse(loki::pddl::NumericFluent numeric_fluent, PDDLFactories& factories)
{
    return factories.numeric_fluents.get_or_create<NumericFluentImpl>(parse(numeric_fluent->get_function(), factories), numeric_fluent->get_number());
}

NumericFluentList parse(loki::pddl::NumericFluentList numeric_fluent_list, PDDLFactories& factories)
{
    auto result_numeric_fluent_list = NumericFluentList();
    for (const auto& numeric_fluent : numeric_fluent_list)
    {
        result_numeric_fluent_list.push_back(parse(numeric_fluent, factories));
    }
    return result_numeric_fluent_list;
}
}
