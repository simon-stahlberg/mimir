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

ConditionVisitor::ConditionVisitor(PDDLFactories& factories_) : factories(factories_) {}

Condition ConditionVisitor::operator()(const loki::ConditionLiteralImpl& node)
{
    return factories.get_or_create_condition_literal(parse(node.get_literal(), factories));
}

Condition ConditionVisitor::operator()(const loki::ConditionAndImpl& node)
{
    return factories.get_or_create_condition_and(parse(node.get_conditions(), factories));
}

Condition ConditionVisitor::operator()(const loki::ConditionOrImpl& node)
{
    return factories.get_or_create_condition_or(parse(node.get_conditions(), factories));
}

Condition ConditionVisitor::operator()(const loki::ConditionNotImpl& node)
{
    return factories.get_or_create_condition_not(parse(node.get_condition(), factories));
}

Condition ConditionVisitor::operator()(const loki::ConditionImplyImpl& node)
{
    return factories.get_or_create_condition_imply(parse(node.get_condition_left(), factories), parse(node.get_condition_right(), factories));
}

Condition ConditionVisitor::operator()(const loki::ConditionExistsImpl& node)
{
    return factories.get_or_create_condition_exists(parse(node.get_parameters(), factories), parse(node.get_condition(), factories));
}

Condition ConditionVisitor::operator()(const loki::ConditionForallImpl& node)
{
    return factories.get_or_create_condition_forall(parse(node.get_parameters(), factories), parse(node.get_condition(), factories));
}

Condition parse(loki::Condition condition, PDDLFactories& factories) { return std::visit(ConditionVisitor(factories), *condition); }

ConditionList parse(loki::ConditionList condition_list, PDDLFactories& factories)
{
    auto result_condition_list = ConditionList();
    for (const auto& condition : condition_list)
    {
        result_condition_list.push_back(parse(condition, factories));
    }
    return result_condition_list;
}
}
