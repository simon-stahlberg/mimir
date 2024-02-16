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

#include <mimir/formalism/domain/parsers/effects.hpp>

#include <mimir/formalism/domain/parsers/conditions.hpp>
#include <mimir/formalism/domain/parsers/literal.hpp>
#include <mimir/formalism/domain/parsers/parameter.hpp>
#include <mimir/formalism/domain/parsers/function.hpp>
#include <mimir/formalism/domain/parsers/function_expressions.hpp>


namespace mimir 
{
    EffectVisitor::EffectVisitor(PDDLFactories& factories_) : factories(factories_) {}

    Effect EffectVisitor::operator()(const loki::pddl::EffectLiteralImpl& node)
    {
        return factories.effects.get_or_create<EffectLiteralImpl>(parse(node.get_literal(), factories));
    }

    Effect EffectVisitor::operator()(const loki::pddl::EffectAndImpl& node)
    {
        return factories.effects.get_or_create<EffectAndImpl>(parse(node.get_effects(), factories));
    }

    Effect EffectVisitor::operator()(const loki::pddl::EffectNumericImpl& node)
    {
        return factories.effects.get_or_create<EffectNumericImpl>(
            node.get_assign_operator(),
            parse(node.get_function(), factories),
            parse(node.get_function_expression(), factories));
    }

    Effect EffectVisitor::operator()(const loki::pddl::EffectConditionalForallImpl& node)
    {
        return factories.effects.get_or_create<EffectConditionalForallImpl>(
            parse(node.get_parameters(), factories),
            parse(node.get_effect(), factories));
    }

    Effect EffectVisitor::operator()(const loki::pddl::EffectConditionalWhenImpl& node)
    {
        return factories.effects.get_or_create<EffectConditionalWhenImpl>(
            parse(node.get_condition(), factories),
            parse(node.get_effect(), factories));
    }


    Effect parse(loki::pddl::Effect effect, PDDLFactories& factories) 
    {
        return std::visit(EffectVisitor(factories), *effect);
    }

    EffectList parse(loki::pddl::EffectList effect_list, PDDLFactories& factories)
    {
        auto result_effect_list = EffectList();
        for (const auto& effect : effect_list) {
            result_effect_list.push_back(parse(effect, factories));
        }
        return result_effect_list;
    }
}
