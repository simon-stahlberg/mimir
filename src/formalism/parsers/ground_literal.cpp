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

#include "ground_literal.hpp"

#include "ground_atom.hpp"
#include "mimir/formalism/factories.hpp"

namespace mimir
{
GroundLiteral parse(loki::pddl::GroundLiteral ground_literal, PDDLFactories& factories)
{
    return factories.ground_literals.get_or_create<GroundLiteralImpl>(ground_literal->is_negated(), parse(ground_literal->get_atom(), factories));
}

GroundLiteralList parse(loki::pddl::GroundLiteralList ground_literal_list, PDDLFactories& factories)
{
    auto result_ground_literal_list = GroundLiteralList();
    for (const auto& ground_literal : ground_literal_list)
    {
        result_ground_literal_list.push_back(parse(ground_literal, factories));
    }
    return result_ground_literal_list;
}
}
