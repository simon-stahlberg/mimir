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

#include <mimir/formalism/domain/parsers/predicate.hpp>

#include <mimir/formalism/domain/parsers/parameter.hpp>


namespace mimir 
{
    Predicate parse(loki::pddl::Predicate predicate, PDDLFactories& factories) {
        std::cout << "Predicate parse" << std::endl;
        return factories.predicates.get_or_create<PredicateImpl>(predicate->get_name(), parse(predicate->get_parameters(), factories));
    }

    PredicateList parse(loki::pddl::PredicateList predicate_list, PDDLFactories& factories) {
        std::cout << "Predicates parse" << std::endl;
        auto result_predicate_list = PredicateList();
        for (const auto& predicate : predicate_list) {
            result_predicate_list.push_back(parse(predicate, factories));
        }
        return result_predicate_list;
    }
}
