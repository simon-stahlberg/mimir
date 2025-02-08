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

#ifndef MIMIR_SEARCH_MATCH_TREE_MATCH_TREE_HPP_
#define MIMIR_SEARCH_MATCH_TREE_MATCH_TREE_HPP_

#include "mimir/formalism/ground_action.hpp"
#include "mimir/formalism/ground_axiom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/construction_helpers/queue_entry.hpp"
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/nodes/atom.hpp"
#include "mimir/search/match_tree/nodes/generator.hpp"
#include "mimir/search/match_tree/nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/queue_entry_scoring_functions/min_depth.hpp"
#include "mimir/search/match_tree/split_scoring_functions/frequency.hpp"

#include <queue>
#include <vector>

namespace mimir::match_tree
{
/* MatchTree */
template<HasConjunctiveCondition Element>
class MatchTree
{
private:
    std::vector<const Element*> m_elements;  ///< ATTENTION: must remain persistent. Swapping elements is allowed.
    Node<Element> m_root;                    ///< the root node.

    std::vector<const INode<Element>*> m_evaluate_stack;  ///< temporary during evaluation.

    template<DynamicPredicateTag P>
    bool contains_positive(GroundAtom<P> atom, const Element* element)
    {
        const auto& conjunctive_condition = element->get_conjunctive_condition();
        const auto& positive_precondition = conjunctive_condition.template get_positive_precondition<P>();
        return (std::find(positive_precondition.begin(), positive_precondition.end(), atom->get_index()) != positive_precondition.end());
    }

    template<DynamicPredicateTag P>
    bool contains_negative(GroundAtom<P> atom, const Element* element)
    {
        const auto& conjunctive_condition = element->get_conjunctive_condition();
        const auto& negative_precondition = conjunctive_condition.template get_negative_precondition<P>();
        return (std::find(negative_precondition.begin(), negative_precondition.end(), atom->get_index()) != negative_precondition.end());
    }

    bool contains(GroundNumericConstraint constraint, const Element* element)
    {
        const auto& conjunctive_condition = element->get_conjunctive_condition();
        const auto& numeric_constraints = conjunctive_condition.get_numeric_constraints();
        return (std::find(numeric_constraints.begin(), numeric_constraints.end(), constraint) != numeric_constraints.end());
    }

    /*
        template<DynamicPredicateTag P>
        Node<Element> instantiate_node_and_update_queue(const SplitList& past_splits, GroundAtom<P> atom, std::span<const Element*> elements)
        {
            // swap true to front
            auto num_true = size_t(0);
            for (size_t i = 0; i < elements.size(); ++i)
            {
                const auto& element = elements[i];
                if (contains_positive(atom, element))
                {
                    std::swap(elements[i], elements[num_true++]);
                }
            }
            std::cout << "num_true: " << num_true;

            auto num_false = size_t(0);
            for (size_t i = num_true; i < elements.size(); ++i)
            {
                const auto& element = elements[i];
                if (contains_negative(atom, element))
                {
                    std::swap(elements[i], elements[num_false++]);
                }
            }
        }

        template<DynamicPredicateTag P>
        Node<Element>
        instantiate_node_and_update_queue(const SplitList& past_splits, GroundNumericConstraint split, std::span<const Element*> elements, QueueType& queue)
        {
        }

        Node<Element> instantiate_node_and_update_queue(const SplitList& past_splits, const Split& split, std::span<const Element*> elements, QueueType& queue)
        {
            return std::visit([&](auto&& arg) { return instantiate_node_and_update_queue(past_splits, arg, elements, queue); }, split);
        }
        */

    InverseNode<Element> build_root(const SplitScoringFunction<Element>& split_scoring_function)
    {
        // Dummy node to represent empty split history.
        auto dummy_parent = std::make_shared<InverseAtomSelectorNode<Element, Fluent>>(nullptr,
                                                                                       0.,
                                                                                       nullptr,
                                                                                       std::span<const Element*>(),
                                                                                       std::span<const Element*>(),
                                                                                       std::span<const Element*>());

        // Span over all elements
        auto root_span = std::span<const Element*>(m_elements.begin(), m_elements.end());

        auto root_split = split_scoring_function->compute_best_split(dummy_parent, root_span);
        std::cout << "root_split: " << root_split << std::endl;

        return nullptr;
    }

    void build_iteratively(const QueueEntryScoringFunction<Element>& queue_scoring_function, const SplitScoringFunction<Element>& split_scoring_function)
    {
        if (m_elements.empty())
        {
            // TODO: create an empty generator node.
            return;
        }

        struct QueueEntryComparator
        {
            bool operator()(const InverseNode<Element>& lhs, const InverseNode<Element>& rhs) const { return lhs->get_queue_score() > rhs->get_queue_score(); }
        };

        auto queue = std::priority_queue<InverseNode<Element>, std::vector<InverseNode<Element>>, QueueEntryComparator> {};
        queue.emplace(build_root(split_scoring_function));

        while (!queue.empty()) {}
    }

public:
    MatchTree(std::vector<const Element*> elements,  //
              const QueueEntryScoringFunction<Element>& queue_scoring_function,
              const SplitScoringFunction<Element>& split_scoring_function) :
        m_elements(std::move(elements))
    {
        build_iteratively(queue_scoring_function, split_scoring_function);
    }
    // Uncopieable and unmoveable to prohibit invalidating spans on m_elements.
    MatchTree(const MatchTree& other) = delete;
    MatchTree& operator=(const MatchTree& other) = delete;
    MatchTree(MatchTree&& other) = delete;
    MatchTree& operator=(MatchTree&& other) = delete;

    void generate_applicable_elements_iteratively(const DenseState& state, std::vector<const Element*>& out_applicable_elements)
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
};

}

#endif
