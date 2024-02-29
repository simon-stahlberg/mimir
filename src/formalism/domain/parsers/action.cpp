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
#include <mimir/formalism/domain/parsers/conditions.hpp>
#include <mimir/formalism/domain/parsers/effects.hpp>
#include <mimir/formalism/domain/parsers/parameter.hpp>

namespace mimir
{
Action parse(loki::pddl::Action action, PDDLFactories& factories)
{
    return factories.actions.get_or_create<ActionImpl>(
        action->get_name(),
        parse(action->get_parameters(), factories),
        (action->get_condition().has_value() ? std::optional<Condition>(parse(action->get_condition().value(), factories)) : std::nullopt),
        (action->get_effect().has_value() ? std::optional<Effect>(parse(action->get_effect().value(), factories)) : std::nullopt));
}

ActionList parse(loki::pddl::ActionList action_list, PDDLFactories& factories)
{
    auto result_action_list = ActionList();
    for (const auto& action : action_list)
    {
        result_action_list.push_back(parse(action, factories));
    }
    return result_action_list;
}
}
