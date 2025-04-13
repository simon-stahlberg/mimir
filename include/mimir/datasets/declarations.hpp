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

#ifndef MIMIR_DATASETS_DECLARATIONS_HPP_
#define MIMIR_DATASETS_DECLARATIONS_HPP_

// Do not include headers with transitive dependencies.
#include "mimir/graphs/algorithms/nauty.hpp"
#include "mimir/graphs/types.hpp"
#include "mimir/search/declarations.hpp"

#include <cstdint>
#include <vector>

namespace mimir::datasets
{
class KnowledgeBaseImpl;
using KnowledgeBase = std::shared_ptr<KnowledgeBaseImpl>;

class StateSpaceImpl;
using StateSpace = std::shared_ptr<StateSpaceImpl>;
using StateSpaceList = std::vector<StateSpace>;

class GeneralizedStateSpaceImpl;
using GeneralizedStateSpace = std::shared_ptr<GeneralizedStateSpaceImpl>;

class TupleGraphImpl;
using TupleGraph = std::shared_ptr<TupleGraphImpl>;
using TupleGraphList = std::vector<TupleGraph>;

template<typename Value>
using CertificateMap = UnorderedMap<graphs::nauty::SparseGraph, Value>;

template<typename Key>
using ToCertificateMap = UnorderedMap<Key, graphs::nauty::SparseGraph>;

struct CertificateMaps
{
    ToCertificateMap<search::State> state_to_cert;
    CertificateMap<graphs::VertexIndex> cert_to_v_idx;
};

using CertificateMapsList = std::vector<CertificateMaps>;

}

#endif
