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

#include "type.hpp"

#include "mimir/formalism/factories.hpp"

namespace mimir
{
Type parse(loki::Type type, PDDLFactories& factories) { return factories.get_or_create_type(type->get_name(), parse(type->get_bases(), factories)); }

TypeList parse(loki::TypeList type_list, PDDLFactories& factories)
{
    auto result_type_list = TypeList();
    for (const auto& type : type_list)
    {
        result_type_list.push_back(parse(type, factories));
    }
    return result_type_list;
}
}
