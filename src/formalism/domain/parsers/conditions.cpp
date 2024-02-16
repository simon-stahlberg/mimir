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

#include <mimir/formalism/domain/parsers/conditions.hpp>

#include <mimir/formalism/domain/parsers/literal.hpp>
#include <mimir/formalism/domain/parsers/parameter.hpp>


namespace mimir 
{

    ConditionVisitor::ConditionVisitor(PDDLFactories& factories_) : factories(factories_) {}

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionLiteralImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionLiteralImpl>(
            parse(node.get_literal(), factories));
    }

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionAndImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionAndImpl>(
            parse(node.get_conditions(), factories));
    }

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionOrImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionOrImpl>(
            parse(node.get_conditions(), factories));
    }

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionNotImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionNotImpl>(
            parse(node.get_condition(), factories));
    }

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionImplyImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionImplyImpl>(
            parse(node.get_condition_left(), factories),
            parse(node.get_condition_right(), factories));
    }

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionExistsImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionExistsImpl>(
            parse(node.get_parameters(), factories),
            parse(node.get_condition(), factories));
    }

    Condition ConditionVisitor::operator()(const loki::pddl::ConditionForallImpl& node)
    {
        std::cout << "Condition parse" << std::endl;
        return factories.conditions.get_or_create<ConditionForallImpl>(
            parse(node.get_parameters(), factories),
            parse(node.get_condition(), factories));
    }


    Condition parse(loki::pddl::Condition condition, PDDLFactories& factories)
    {
        std::cout << "Condition parse" << std::endl;
        return std::visit(ConditionVisitor(factories), *condition);
    }

    ConditionList parse(loki::pddl::ConditionList condition_list, PDDLFactories& factories)
    {
        std::cout << "Conditions parse" << std::endl;
        auto result_condition_list = ConditionList();
        for (const auto& condition : condition_list) {
            result_condition_list.push_back(parse(condition, factories));
        }
        return result_condition_list;
    }
}
