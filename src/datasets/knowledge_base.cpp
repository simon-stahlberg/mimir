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

#include "mimir/datasets/knowledge_base.hpp"

#include "mimir/datasets/generalized_state_space.hpp"
#include "mimir/datasets/state_space.hpp"
#include "mimir/datasets/tuple_graph.hpp"

using namespace mimir::formalism;
using namespace mimir::search;

namespace mimir::datasets
{
KnowledgeBaseImpl::KnowledgeBaseImpl(formalism::Domain domain,
                                     StateSpaceList state_spaces,
                                     std::optional<GeneralizedStateSpace> generalized_state_space,
                                     std::optional<std::vector<TupleGraphList>> tuple_graphs) :
    m_domain(std::move(domain)),
    m_state_spaces(std::move(state_spaces)),
    m_generalized_state_space(std::move(generalized_state_space)),
    m_tuple_graphs(std::move(tuple_graphs))
{
}

/**
 * Constructors
 */

KnowledgeBase KnowledgeBaseImpl::create(search::GeneralizedSearchContext contexts, const Options& options)
{
    auto state_spaces = StateSpaceImpl::create(contexts, options.state_space_options);

    auto generalized_state_space = std::optional<GeneralizedStateSpace> { std::nullopt };
    if (options.generalized_state_space_options)
    {
        generalized_state_space = GeneralizedStateSpaceImpl::create(state_spaces, options.generalized_state_space_options.value());

        state_spaces = generalized_state_space.value()->get_state_spaces();  ///< update state spaces after potential symmetry pruning
    }

    auto tuple_graphs = std::optional<std::vector<TupleGraphList>> { std::nullopt };
    if (options.tuple_graph_options)
    {
        auto tmp_tuple_graphs = std::vector<TupleGraphList> {};
        for (const auto& state_space : state_spaces)
        {
            tmp_tuple_graphs.push_back(TupleGraphImpl::create(state_space, options.tuple_graph_options.value()));
        }
        tuple_graphs = std::move(tmp_tuple_graphs);
    }

    return std::make_shared<KnowledgeBaseImpl>(contexts.get_domain(), std::move(state_spaces), std::move(generalized_state_space), std::move(tuple_graphs));
}

/**
 * Getters
 */

const formalism::Domain& KnowledgeBaseImpl::get_domain() const { return m_domain; }
const StateSpaceList& KnowledgeBaseImpl::get_state_spaces() const { return m_state_spaces; }
const std::optional<GeneralizedStateSpace>& KnowledgeBaseImpl::get_generalized_state_space() const { return m_generalized_state_space; }
const std::optional<std::vector<TupleGraphList>>& KnowledgeBaseImpl::get_tuple_graphs() const { return m_tuple_graphs; }

}