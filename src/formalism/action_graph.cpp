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

#include "mimir/formalism/action_graph.hpp"

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/parameter.hpp"

namespace mimir::formalism
{

static VariableMap<graphs::VertexIndex> add_variables_graph_structures(const ActionImpl& action)
{
    using VariableColorData = std::vector<std::pair<PredicateVariant, bool>>;

    auto variable_colors = VariableMap<VariableColorData> {};

    // --- Step 1: initialize all variables appearing in the action ---

    auto initialize_variable_colors_func = [&](auto&& parameter)
    {
        // default-construct VariableColorData if not present
        (void) variable_colors[parameter->get_variable()];
    };

    for (const auto& parameter : action.get_conjunctive_condition()->get_parameters())
    {
        initialize_variable_colors_func(parameter);
    }

    for (const auto& cond_effect : action.get_conditional_effects())
    {
        for (const auto& parameter : cond_effect->get_conjunctive_condition()->get_parameters())
        {
            initialize_variable_colors_func(parameter);
        }
    }

    // --- Step 2: fill unary colors ---
}

graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>> create_action_graph(const ActionImpl& action)
{
    auto vertex_colored_digraph = graphs::StaticGraph<graphs::Vertex<graphs::PropertyValue>, graphs::Edge<>>();

    const auto variable_to_vertex_index = add_variables_graph_structures(action);
}
}
