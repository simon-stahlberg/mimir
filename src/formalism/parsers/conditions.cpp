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

#include "conditions.hpp"

#include "literal.hpp"
#include "mimir/formalism/factories.hpp"
#include "parameter.hpp"

namespace mimir
{

std::pair<ParameterList, LiteralList> parse(loki::Condition condition, PDDLFactories& factories)
{
    auto parameters = ParameterList {};
    if (const auto condition_exists = std::get_if<loki::ConditionExistsImpl>(condition))
    {
        parameters = parse(condition_exists->get_parameters(), factories);

        condition = condition_exists->get_condition();
    }

    if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(condition))
    {
        auto literals = LiteralList {};
        for (const auto& part : condition_and->get_conditions())
        {
            if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
            {
                literals.push_back(parse(condition_literal->get_literal(), factories));
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return std::make_pair(parameters, literals);
    }
    else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(condition))
    {
        return std::make_pair(parameters, LiteralList { parse(condition_literal->get_literal(), factories) });
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *condition) << std::endl;

    throw std::logic_error("Expected conjunctive condition.");
}

}
