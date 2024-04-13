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

#include "axiom.hpp"

#include "conditions.hpp"
#include "literal.hpp"
#include "mimir/formalism/factories.hpp"

namespace mimir
{
Axiom parse(loki::pddl::Axiom axiom, PDDLFactories& factories)
{
    return factories.get_or_create_axiom(parse(axiom->get_literal(), factories), parse(axiom->get_condition(), factories));
}

extern AxiomList parse(loki::pddl::AxiomList axiom_list, PDDLFactories& factories)
{
    auto result_axiom_list = AxiomList();
    for (const auto& axiom : axiom_list)
    {
        result_axiom_list.push_back(parse(axiom, factories));
    }
    return result_axiom_list;
}
}
