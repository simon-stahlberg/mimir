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

namespace mimir
{
KnowledgeBase::KnowledgeBase(SearchContextList contexts, const Options& options) :
    m_state_space(contexts, options.state_space_options),
    m_tuple_graphs(options.tuple_graph_options ? std::optional<TupleGraphCollection>(TupleGraphCollection(m_state_space, options.tuple_graph_options.value())) :
                                                 std::nullopt)
{
}
KnowledgeBase::KnowledgeBase(SearchContext context, const Options& options) :
    m_state_space(context, options.state_space_options),
    m_tuple_graphs(options.tuple_graph_options ? std::optional<TupleGraphCollection>(TupleGraphCollection(m_state_space, options.tuple_graph_options.value())) :
                                                 std::nullopt)
{
}

std::unique_ptr<KnowledgeBase> KnowledgeBase::create(SearchContextList contexts, const Options& options)
{
    return std::make_unique<KnowledgeBase>(contexts, options);
}

std::unique_ptr<KnowledgeBase> KnowledgeBase::create(SearchContext context, const Options& options)
{
    return std::make_unique<KnowledgeBase>(context, options);
}

/**
 * Getters
 */

const GeneralizedStateSpace& KnowledgeBase::get_generalized_state_space() const { return m_state_space; }

const std::optional<TupleGraphCollection>& KnowledgeBase::get_tuple_graphs() const { return m_tuple_graphs; }
}