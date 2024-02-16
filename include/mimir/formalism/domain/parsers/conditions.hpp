/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef MIMIR_FORMALISM_DOMAIN_PARSER_CONDITIONS_HPP_
#define MIMIR_FORMALISM_DOMAIN_PARSER_CONDITIONS_HPP_

#include "../conditions.hpp"
#include "../declarations.hpp"
#include "../../common/types.hpp"

#include <variant>


namespace mimir 
{
    class ConditionVisitor {
        private:
            PDDLFactories& factories;

        public:
            ConditionVisitor(PDDLFactories& factories_);

            Condition operator()(const loki::pddl::ConditionLiteralImpl& node);
            Condition operator()(const loki::pddl::ConditionAndImpl& node);
            Condition operator()(const loki::pddl::ConditionOrImpl& node);
            Condition operator()(const loki::pddl::ConditionNotImpl& node);
            Condition operator()(const loki::pddl::ConditionImplyImpl& node);
            Condition operator()(const loki::pddl::ConditionExistsImpl& node);
            Condition operator()(const loki::pddl::ConditionForallImpl& node);
    };

    extern Condition parse(loki::pddl::Condition condition, PDDLFactories& factories);
    extern ConditionList parse(loki::pddl::ConditionList condition_list, PDDLFactories& factories);
} 


#endif 
