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

#include "effect.hpp"

#include "condition.hpp"
#include "function.hpp"
#include "function_expressions.hpp"
#include "literal.hpp"
#include "mimir/formalism/factories.hpp"
#include "parameter.hpp"

namespace mimir
{

EffectList parse_simple_effects(loki::Effect effect, PDDLFactories& factories)
{
    // 1. Parse conjunctive part
    if (const auto& effect_and = std::get_if<loki::EffectAndImpl>(effect))
    {
        auto result = EffectList {};
        for (const auto& nested_effect : effect_and->get_effects())
        {
            auto tmp_effect = nested_effect;

            // 2. Parse universal part
            auto parameters = ParameterList {};
            if (const auto& tmp_effect_forall = std::get_if<loki::EffectConditionalForallImpl>(tmp_effect))
            {
                parameters = parse(tmp_effect_forall->get_parameters(), factories);

                tmp_effect = tmp_effect_forall->get_effect();
            }

            // 3. Parse conditional part
            auto conditions = LiteralList {};
            if (const auto& tmp_effect_when = std::get_if<loki::EffectConditionalWhenImpl>(tmp_effect))
            {
                if (const auto condition_and = std::get_if<loki::ConditionAndImpl>(tmp_effect_when->get_condition()))
                {
                    for (const auto& part : condition_and->get_conditions())
                    {
                        if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(part))
                        {
                            conditions.push_back(parse(condition_literal->get_literal(), factories));
                        }
                        else
                        {
                            std::cout << std::visit([](auto&& arg) { return arg.str(); }, *part) << std::endl;

                            throw std::logic_error("Expected literal in conjunctive condition.");
                        }
                    }
                }
                else if (const auto condition_literal = std::get_if<loki::ConditionLiteralImpl>(tmp_effect_when->get_condition()))
                {
                    conditions = LiteralList { parse(condition_literal->get_literal(), factories) };
                }

                tmp_effect = tmp_effect_when->get_effect();
            }

            // 4. Parse simple effect
            if (const auto& effect_literal = std::get_if<loki::EffectLiteralImpl>(tmp_effect))
            {
                result.push_back(factories.get_or_create_simple_effect(parameters, conditions, parse(effect_literal->get_literal(), factories)));
            }
            else if (const auto& effect_numeric = std::get_if<loki::EffectNumericImpl>(tmp_effect))
            {
                // TODO: implement how we should handle numeric effect
            }
            else
            {
                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *effect) << std::endl;

                std::cout << std::visit([](auto&& arg) { return arg.str(); }, *tmp_effect) << std::endl;

                throw std::logic_error("Expected simple effect.");
            }
        }

        return result;
    }

    std::cout << std::visit([](auto&& arg) { return arg.str(); }, *effect) << std::endl;

    throw std::logic_error("Expected conjunctive effect.");
}
}
