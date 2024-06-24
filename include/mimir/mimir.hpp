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

#ifndef MIMIR_MIMIR_HPP_
#define MIMIR_MIMIR_HPP_

/**
 * Include all specializations here
 */

/**
 * Formalism
 */

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/variable.hpp"

/**
 * Search
 */

#include "mimir/search/action.hpp"
#include "mimir/search/algorithms.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/openlists.hpp"
#include "mimir/search/planners.hpp"
#include "mimir/search/search_nodes.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/successor_state_generator.hpp"

/**
 * DataSet
 */

#include "mimir/datasets/faithful_abstraction.hpp"
#include "mimir/datasets/global_faithful_abstraction.hpp"
#include "mimir/datasets/state_space.hpp"

/**
 * Graphs
 */

#include "mimir/graphs/coloring.hpp"
#include "mimir/graphs/digraph.hpp"
#include "mimir/graphs/object_graph.hpp"
#include "mimir/graphs/tuple_graph.hpp"

/**
 * Algorithms
 */

#include "mimir/algorithms/nauty.hpp"

#endif  // MIMIR_MIMIR_HPP_
