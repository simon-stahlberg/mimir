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

#include <mimir/formalism/domain/parsers/literal.hpp>

#include <mimir/formalism/domain/parsers/atom.hpp>


namespace mimir 
{
    Literal parse(loki::pddl::Literal literal, PDDLFactories& factories) {
        std::cout << "Literal parse" << std::endl;
        return factories.literals.get_or_create<LiteralImpl>(literal->is_negated(), parse(literal->get_atom(), factories));
    }

    LiteralList parse(loki::pddl::LiteralList literal_list, PDDLFactories& factories) {
        std::cout << "Literals parse" << std::endl;
        auto result_literal_list = LiteralList();
        for (const auto& literal : literal_list) {
            result_literal_list.push_back(parse(literal, factories));
        }
        return result_literal_list;
    }
}
