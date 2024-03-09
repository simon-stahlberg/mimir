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

#include <mimir/formalism/factories.hpp>
#include <mimir/formalism/parsers/object.hpp>
#include <mimir/formalism/parsers/type.hpp>

namespace mimir
{
Object parse(loki::pddl::Object object, PDDLFactories& factories)
{
    return factories.objects.get_or_create<ObjectImpl>(object->get_name(), parse(object->get_bases(), factories));
}

ObjectList parse(loki::pddl::ObjectList object_list, PDDLFactories& factories)
{
    auto result_object_list = ObjectList();
    for (const auto& object : object_list)
    {
        result_object_list.push_back(parse(object, factories));
    }
    return result_object_list;
}
}
