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

#include "condition.hpp"
#include "literal.hpp"
#include "mimir/formalism/factories.hpp"
#include "mimir/formalism/translators/utils.hpp"
#include "parameter.hpp"
#include "variable.hpp"

namespace mimir
{
Axiom parse(loki::Axiom axiom, PDDLFactories& factories)
{
    // Turn predicate arguments into parameters
    auto parameters = parse(axiom->get_literal()->get_atom()->get_predicate()->get_parameters(), factories);

    // Turn quantifier variables into parameters
    const auto [additional_parameters, literals] = parse(axiom->get_condition(), factories);
    parameters.insert(parameters.end(), additional_parameters.begin(), additional_parameters.end());

    // Turn free variables into parameters
    for (const auto& free_variable : collect_free_variables(*axiom->get_condition()))
    {
        parameters.push_back(factories.get_or_create_parameter(parse(free_variable, factories)));
    }
    parameters.shrink_to_fit();

    return factories.get_or_create_axiom(parameters, parse(axiom->get_literal(), factories), literals);
}

extern AxiomList parse(loki::AxiomList axiom_list, PDDLFactories& factories)
{
    auto result_axiom_list = AxiomList();
    for (const auto& axiom : axiom_list)
    {
        result_axiom_list.push_back(parse(axiom, factories));
    }
    return result_axiom_list;
}
}
