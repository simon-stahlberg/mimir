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

#ifndef MIMIR_FORMALISM_PARSER_CONDITIONS_HPP_
#define MIMIR_FORMALISM_PARSER_CONDITIONS_HPP_

#include "mimir/formalism/conditions.hpp"
#include "mimir/formalism/declarations.hpp"

#include <variant>

namespace mimir
{
class ConditionVisitor
{
private:
    PDDLFactories& factories;

public:
    ConditionVisitor(PDDLFactories& factories_);

    Condition operator()(const loki::ConditionLiteralImpl& node);
    Condition operator()(const loki::ConditionAndImpl& node);
    Condition operator()(const loki::ConditionOrImpl& node);
    Condition operator()(const loki::ConditionNotImpl& node);
    Condition operator()(const loki::ConditionImplyImpl& node);
    Condition operator()(const loki::ConditionExistsImpl& node);
    Condition operator()(const loki::ConditionForallImpl& node);
};

extern Condition parse(loki::Condition condition, PDDLFactories& factories);
extern ConditionList parse(loki::ConditionList condition_list, PDDLFactories& factories);
}

#endif
