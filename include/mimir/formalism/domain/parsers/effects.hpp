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

#ifndef MIMIR_FORMALISM_DOMAIN_PARSER_EFFECTS_HPP_
#define MIMIR_FORMALISM_DOMAIN_PARSER_EFFECTS_HPP_

#include "../../common/types.hpp"
#include "../conditions.hpp"
#include "../declarations.hpp"

#include <variant>

namespace mimir
{
class EffectVisitor
{
private:
    PDDLFactories& factories;

public:
    EffectVisitor(PDDLFactories& factories_);

    Effect operator()(const loki::pddl::EffectLiteralImpl& node);
    Effect operator()(const loki::pddl::EffectAndImpl& node);
    Effect operator()(const loki::pddl::EffectNumericImpl& node);
    Effect operator()(const loki::pddl::EffectConditionalForallImpl& node);
    Effect operator()(const loki::pddl::EffectConditionalWhenImpl& node);
};

extern Effect parse(loki::pddl::Effect effect, PDDLFactories& factories);
extern EffectList parse(loki::pddl::EffectList effect_list, PDDLFactories& factories);
}

#endif
