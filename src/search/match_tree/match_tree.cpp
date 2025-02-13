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
#include "mimir/search/match_tree/node_splitters/static.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"

#include <queue>

namespace mimir::match_tree
{

/* MatchTree */

template<HasConjunctiveCondition Element>
MatchTree<Element>::MatchTree() : m_elements(), m_options(), m_root(create_root_generator_node(std::span<const Element*>(m_elements.begin(), m_elements.end())))
{
    m_statistics.generator_distribution.push_back(0);
}

template<HasConjunctiveCondition Element>
MatchTree<Element>::MatchTree(const PDDLRepositories& pddl_repositories, std::vector<const Element*> elements, const Options& options) :
    m_elements(std::move(elements)),
    m_options(options),
    m_root(create_root_generator_node(std::span<const Element*>(m_elements.begin(), m_elements.end())))
{
    if (!m_elements.empty())
    {
        auto node_splitter = NodeSplitter<Element> { nullptr };
        switch (m_options.split_strategy)
        {
            case SplitStrategyEnum::STATIC:
            {
                node_splitter = std::make_unique<StaticNodeSplitter<Element>>(pddl_repositories,
                                                                              m_options,
                                                                              std::span<const Element*>(m_elements.begin(), m_elements.end()));
                break;
            }
            case SplitStrategyEnum::HYBRID:
            {
                throw std::runtime_error("Not implemented.");
            }
            case SplitStrategyEnum::DYNAMIC:
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

template<HasConjunctiveCondition Element>
void MatchTree<Element>::generate_applicable_elements_iteratively(const DenseState& state, std::vector<const Element*>& out_applicable_elements)
{
    m_evaluate_stack.clear();
    out_applicable_elements.clear();

    m_evaluate_stack.push_back(m_root.get());

    while (!m_evaluate_stack.empty())
    {
        const auto node = m_evaluate_stack.back();

        m_evaluate_stack.pop_back();

        node->generate_applicable_actions(state, m_evaluate_stack, out_applicable_elements);
    }
}

template<HasConjunctiveCondition Element>
const Statistics& MatchTree<Element>::get_statistics() const
{
    return m_statistics;
}

template<HasConjunctiveCondition Element>
std::unique_ptr<MatchTree<Element>>
MatchTree<Element>::create(const PDDLRepositories& pddl_repositories, std::vector<const Element*> elements, const Options& options)
{
    return std::unique_ptr<MatchTree<Element>>(new MatchTree<Element>(pddl_repositories, std::move(elements), options));
}

template class MatchTree<GroundActionImpl>;
template class MatchTree<GroundAxiomImpl>;
}
