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

#include "effects.hpp"

#include "conditions.hpp"
#include "function.hpp"
#include "function_expressions.hpp"
#include "literal.hpp"
#include "mimir/formalism/factories.hpp"
#include "parameter.hpp"

namespace mimir
{
EffectVisitor::EffectVisitor(PDDLFactories& factories_) : factories(factories_) {}

Effect EffectVisitor::operator()(const loki::EffectLiteralImpl& node) { return factories.get_or_create_effect_literal(parse(node.get_literal(), factories)); }

Effect EffectVisitor::operator()(const loki::EffectAndImpl& node) { return factories.get_or_create_effect_and(parse(node.get_effects(), factories)); }

Effect EffectVisitor::operator()(const loki::EffectNumericImpl& node)
{
    return factories.get_or_create_effect_numeric(node.get_assign_operator(),
                                                  parse(node.get_function(), factories),
                                                  parse(node.get_function_expression(), factories));
}

Effect EffectVisitor::operator()(const loki::EffectConditionalForallImpl& node)
{
    return factories.get_or_create_effect_conditional_forall(parse(node.get_parameters(), factories), parse(node.get_effect(), factories));
}

Effect EffectVisitor::operator()(const loki::EffectConditionalWhenImpl& node)
{
    return factories.get_or_create_effect_conditional_when(parse(node.get_condition(), factories), parse(node.get_effect(), factories));
}

Effect parse(loki::Effect effect, PDDLFactories& factories) { return std::visit(EffectVisitor(factories), *effect); }

EffectList parse(loki::EffectList effect_list, PDDLFactories& factories)
{
    auto result_effect_list = EffectList();
    for (const auto& effect : effect_list)
    {
        result_effect_list.push_back(parse(effect, factories));
    }
    return result_effect_list;
}
}
