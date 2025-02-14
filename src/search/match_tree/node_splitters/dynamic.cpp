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

#include "mimir/search/match_tree/node_splitters/dynamic.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_node_creation.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/placeholder.hpp"
#include "mimir/search/match_tree/construction_helpers/split_metrics.hpp"
#include "mimir/search/match_tree/node_splitters/base_impl.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"

#include <map>

namespace mimir::match_tree
{

template<HasConjunctiveCondition Element>
DynamicNodeSplitter<Element>::DynamicNodeSplitter(const PDDLRepositories& pddl_repositories, const Options& options, std::span<const Element*> elements) :
    NodeSplitterBase<DynamicNodeSplitter<Element>, Element>(pddl_repositories, options)
{
}

template<HasConjunctiveCondition Element>
InverseNode<Element> DynamicNodeSplitter<Element>::fit_impl(std::span<const Element*> elements, Statistics& ref_statistics)
{
    auto queue = SplitterQueue<Element>();

    auto root_placeholder = create_root_placeholder_node(elements);
    auto root_refinement_data = this->compute_refinement_data(root_placeholder);

    ++ref_statistics.num_nodes;
    if (!root_refinement_data)
    {
        return create_imperfect_generator_node(root_placeholder);
    }

    queue.emplace(std::move(root_placeholder), root_refinement_data.value());

    auto inverse_root = InverseNode<Element> { nullptr };

    while (!queue.empty())
    {
        auto entry = std::move(const_cast<SplitterQueueEntry<Element>&>(queue.top()));
        queue.pop();

        /* Customization point in derived classes: how to select the node and the split? */
        auto [inverse_node_, placeholder_children_] =
            create_node_and_placeholder_children(std::move(entry.node), entry.refinement_data.useless_splits, entry.refinement_data.split);

        ref_statistics.num_nodes += placeholder_children_.size();
        if (inverse_node_)
        {
            inverse_root = std::move(inverse_node_);
        }

        for (auto& child : placeholder_children_)
        {
            auto child_refinement_data = this->compute_refinement_data(child);

            if (!child_refinement_data)
            {
                create_perfect_generator_node(child);
            }
            else
            {
                queue.emplace(std::move(child), child_refinement_data.value());
            }
        }

        if (ref_statistics.num_nodes >= this->m_options.max_num_nodes)
        {
            /* Mark the tree as imperfect and translate the remaining placeholder nodes to generator nodes. */
            while (!queue.empty())
            {
                auto entry = std::move(const_cast<SplitterQueueEntry<Element>&>(queue.top()));
                queue.pop();

                create_imperfect_generator_node(entry.node);
            }
            break;
        }
    }

    assert(inverse_root);

    return std::move(inverse_root);
}

template class DynamicNodeSplitter<GroundActionImpl>;
template class DynamicNodeSplitter<GroundAxiomImpl>;

}
