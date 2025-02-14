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

#ifndef MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_BASE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_NODE_SPLITTERS_BASE_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/match_tree/construction_helpers/split.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element>
struct SplitterQueueEntry
{
    PlaceholderNode<Element> node;

    SplitScoreAndUselessSplits refinement_data;  ///< entries in the queue must have a well-defined next split
};

template<HasConjunctiveCondition Element>
struct SplitterQueueEntryComparator
{
    bool operator()(const SplitterQueueEntry<Element>& lhs, const SplitterQueueEntry<Element>& rhs) const
    {
        return lhs.refinement_data.score < rhs.refinement_data.score;  // highest score first.
    }
};

template<HasConjunctiveCondition Element>
using SplitterQueue = std::priority_queue<SplitterQueueEntry<Element>, std::vector<SplitterQueueEntry<Element>>, SplitterQueueEntryComparator<Element>>;

template<typename Derived_, HasConjunctiveCondition Element>
class NodeSplitterBase : public INodeSplitter<Element>
{
private:
    /// @brief Helper to cast to Derived_.
    constexpr const auto& self() const { return static_cast<const Derived_&>(*this); }
    constexpr auto& self() { return static_cast<Derived_&>(*this); }

protected:
    const PDDLRepositories& m_pddl_repositories;
    const Options& m_options;

    template<DynamicPredicateTag P>
    using AtomDistributions = std::unordered_map<GroundAtom<P>, AtomSplitDistribution>;
    using NumericConstraintDistributions = std::unordered_map<GroundNumericConstraint, NumericConstraintSplitDistribution>;

    /// @brief Compute all possible ways to split the data.
    /// This operation runs in time O(|E|*|A|) where |E| is the number of elements
    /// and |A| is the maxmum number of preconditions in an element in E.
    SplitSet compute_splits(const std::span<const Element*>& elements);

    /// @brief Compute the best split if there exists any that is not useless.
    /// This operation runs in time O(|E|*|A|) where |E| is the number of elements in the node
    /// and |A| is the maxmum number of preconditions in an element in E.
    std::optional<SplitScoreAndUselessSplits> compute_refinement_data(const PlaceholderNode<Element>& node);

public:
    NodeSplitterBase(const PDDLRepositories& pddl_repositories, const Options& options);
    std::pair<Node<Element>, Statistics> fit(std::span<const Element*> elements) override;
};
}

#endif
