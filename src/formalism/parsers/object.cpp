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

#include "object.hpp"

#include "mimir/formalism/factories.hpp"

namespace mimir
{
Object parse(loki::Object object, PDDLFactories& factories)
{
    if (!object->get_bases().empty())
    {
        throw std::logic_error("Expected types to be empty.");
    }
    return factories.get_or_create_object(object->get_name());
}

ObjectList parse(loki::ObjectList object_list, PDDLFactories& factories)
{
    auto result_object_list = ObjectList();
    for (const auto& object : object_list)
    {
        result_object_list.push_back(parse(object, factories));
    }
    return result_object_list;
}
}
