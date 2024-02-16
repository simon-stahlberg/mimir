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

#include <mimir/formalism/domain/parsers/action.hpp>

#include <mimir/formalism/domain/parsers/parameter.hpp>
#include <mimir/formalism/domain/parsers/conditions.hpp>
#include <mimir/formalism/domain/parsers/effects.hpp>


namespace mimir 
{
    Action parse(loki::pddl::Action action, PDDLFactories& factories) {
        return factories.atoms.get_or_create<ActionImpl>(
            action->get_name()
            parse(atom->get_parameters(), factories), 
            parse(atom->get_condition(), factories),
            parse(atom->get_effect(), factories));
    }

    ActionList parse(loki::pddl::ActionList action_list, PDDLFactories& factories) {
        auto result_action_list = AtomList();
        for (const auto& action : action_list) {
            result_action_list.push_back(parse(action, factories));
        }
        return result_action_list;
    }
}
