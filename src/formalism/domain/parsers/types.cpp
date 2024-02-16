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

#include <mimir/formalism/domain/parsers/type.hpp>


namespace mimir 
{
    Type parse(loki::pddl::Type type, PDDLFactories& factories) {
        std::cout << "Type parse" << std::endl;
        return factories.types.get_or_create<TypeImpl>(type->get_name(), parse(type->get_bases(), factories));
    }

    TypeList parse(loki::pddl::TypeList type_list, PDDLFactories& factories) {
        std::cout << "Types parse" << std::endl;
        auto result_type_list = TypeList();
        for (const auto& type : type_list) {
            result_type_list.push_back(parse(type, factories));
        }
        return result_type_list;
    }
}
