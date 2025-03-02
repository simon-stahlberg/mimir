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
 * Common
 */

#include "mimir/common/printers.hpp"
#include "mimir/common/types_cista.hpp"

/**
 * Formalism
 */

#include "mimir/formalism/action.hpp"
#include "mimir/formalism/assignment_set.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/consistency_graph.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/parser.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/problem.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/formalism/term.hpp"
#include "mimir/formalism/translator.hpp"
#include "mimir/formalism/utils.hpp"
#include "mimir/formalism/variable.hpp"

/**
 * Languages
 */

#include "mimir/languages/description_logics.hpp"

/**
 * Search
 */

#include "mimir/search/algorithms.hpp"
#include "mimir/search/algorithms/strategies/goal_strategy.hpp"
#include "mimir/search/algorithms/strategies/pruning_strategy.hpp"
#include "mimir/search/applicable_action_generators.hpp"
#include "mimir/search/axiom_evaluators.hpp"
#include "mimir/search/delete_relaxed_problem_explorator.hpp"
#include "mimir/search/heuristics.hpp"
#include "mimir/search/openlists.hpp"
#include "mimir/search/satisficing_binding_generators.hpp"
#include "mimir/search/search_context.hpp"
#include "mimir/search/search_node.hpp"
#include "mimir/search/state.hpp"
#include "mimir/search/state_repository.hpp"

/**
 * DataSet
 */

#include "mimir/datasets/generalized_color_function.hpp"
#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/knowledge_base.hpp"
#include "mimir/datasets/object_graph.hpp"
#include "mimir/datasets/tuple_graph.hpp"

/**
 * Graphs
 */

#include "graphs.hpp"

/**
 * Algorithms
 */

#include "mimir/algorithms/nauty.hpp"

#endif  // MIMIR_MIMIR_HPP_
