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

#include "action.hpp"

#include "conditions.hpp"
#include "effects.hpp"
#include "literal.hpp"
#include "mimir/formalism/factories.hpp"
#include "parameter.hpp"

namespace mimir
{
Action parse(loki::Action action, PDDLFactories& factories)
{
    auto parameters = parse(action->get_parameters(), factories);
    auto literals = LiteralList {};
    if (action->get_condition().has_value())
    {
        const auto [additional_parameters, parsed_literals] = parse(action->get_condition().value(), factories);
        literals = parsed_literals;
        parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());
    }

    auto effects = SimpleEffectList {};
    if (action->get_effect().has_value())
    {
        effects = parse_simple_effects(action->get_effect().value(), factories);
    }

    return factories.get_or_create_action(action->get_name(), parameters, literals, effects);
}

ActionList parse(loki::ActionList action_list, PDDLFactories& factories)
{
    auto result_action_list = ActionList();
    for (const auto& action : action_list)
    {
        result_action_list.push_back(parse(action, factories));
    }
    return result_action_list;
}
}
