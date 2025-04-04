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

#include "mimir/search/match_tree/match_tree.hpp"

#include "mimir/common/filesystem.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/node_creation.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/dynamic.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"

#include <queue>

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

/* MatchTree */

template<formalism::HasConjunctiveCondition E>
MatchTreeImpl<E>::MatchTreeImpl() : m_elements(), m_options(), m_root(create_root_generator_node(std::span<const E*>(m_elements.begin(), m_elements.end())))
{
    m_statistics.generator_distribution.push_back(0);
}

template<formalism::HasConjunctiveCondition E>
MatchTreeImpl<E>::MatchTreeImpl(const Repositories& pddl_repositories, std::vector<const E*> elements, const Options& options) :
    m_elements(std::move(elements)),
    m_options(options),
    m_root(create_root_generator_node(std::span<const E*>(m_elements.begin(), m_elements.end())))
{
    if (!m_elements.empty())
    {
        auto node_splitter = NodeSplitter<E> { nullptr };
        switch (m_options.split_strategy)
        {
            case SplitStrategyEnum::DYNAMIC:
            {
                node_splitter =
                    std::make_unique<DynamicNodeSplitter<E>>(pddl_repositories, m_options, std::span<const E*>(m_elements.begin(), m_elements.end()));
                break;
            }
            case SplitStrategyEnum::HYBRID:
            {
                throw std::runtime_error("Not implemented.");
            }
            case SplitStrategyEnum::STATIC:
            {
                throw std::runtime_error("Not implemented.");
            }
            default:
            {
                throw std::logic_error("MatchTree::create: Undefined SplitStrategyEnum type.");
            }
        }

        auto [root_, statistics_] = node_splitter->fit(m_elements);
        m_root = std::move(root_);
        m_statistics = std::move(statistics_);
    }
}

template<formalism::HasConjunctiveCondition E>
void MatchTreeImpl<E>::generate_applicable_elements_iteratively(const DenseState& state,
                                                                const ProblemImpl& problem,
                                                                std::vector<const E*>& out_applicable_elements)
{
    m_evaluate_stack.clear();
    out_applicable_elements.clear();

    m_evaluate_stack.push_back(m_root.get());

    while (!m_evaluate_stack.empty())
    {
        const auto node = m_evaluate_stack.back();

        m_evaluate_stack.pop_back();

        node->generate_applicable_actions(state, problem, m_evaluate_stack, out_applicable_elements);
    }
}

template<formalism::HasConjunctiveCondition E>
const Statistics& MatchTreeImpl<E>::get_statistics() const
{
    return m_statistics;
}

template<formalism::HasConjunctiveCondition E>
std::unique_ptr<MatchTreeImpl<E>> MatchTreeImpl<E>::create(const Repositories& pddl_repositories, std::vector<const E*> elements, const Options& options)
{
    return std::unique_ptr<MatchTreeImpl<E>>(new MatchTreeImpl<E>(pddl_repositories, std::move(elements), options));
}

template class MatchTreeImpl<GroundActionImpl>;
template class MatchTreeImpl<GroundAxiomImpl>;
}
