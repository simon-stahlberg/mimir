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

namespace mimir
{

LiteralList parse(loki::Condition node, PDDLFactories& factories)
{
    if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(node))
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
                throw std::logic_error("Expected literal in conjunctive condition.");
            }
        }
        return literals;
    }

    throw std::logic_error("Expected conjunctive condition.");
}

}
