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

#ifndef MIMIR_SRC_DATASETS_TUPLE_GRAPH_FACTORY_
#define MIMIR_SRC_DATASETS_TUPLE_GRAPH_FACTORY_

#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/tuple_graph.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir::datasets
{
extern TupleGraph create_tuple_graph(const graphs::ProblemVertex& problem_vertex,
                                     const StateSpace& state_space,
                                     std::optional<CertificateMaps>& certificate_maps,
                                     const TupleGraphImpl::Options& options);
}

#endif