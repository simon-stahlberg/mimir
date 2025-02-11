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
#include "mimir/search/match_tree/declarations.hpp"
#include "mimir/search/match_tree/node_score_functions/interface.hpp"
#include "mimir/search/match_tree/node_splitters/interface.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"
#include "mimir/search/match_tree/options.hpp"

#include <optional>
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
    Node<Element> m_root;                    ///< The root node.
    size_t m_max_num_nodes;                  ///< The maximum number of nodes allowed in the tree. Might result in imperfect match trees.

    bool m_enable_dump_dot_file;
    fs::path m_output_dot_file;

    /* Statistics */
    size_t m_num_nodes;
    bool m_is_imperfect;

    std::vector<const INode<Element>*> m_evaluate_stack;  ///< temporary during evaluation.

    void build_iteratively(const NodeScoreFunction<Element>& node_score_function, const NodeSplitter<Element>& node_splitter);

    MatchTree();

    MatchTree(std::vector<const Element*> elements,  //
              const NodeScoreFunction<Element>& node_score_function,
              const NodeSplitter<Element>& node_splitter,
              size_t max_num_nodes = std::numeric_limits<size_t>::max(),
              bool enable_dump_dot_file = false,
              fs::path output_dot_file = fs::path("match_tree.dot"));

public:
    static std::unique_ptr<MatchTree<Element>>
    create(const PDDLRepositories& pddl_repositories, std::vector<const Element*> elements, const Options& options = Options());

    // Uncopieable and unmoveable to prohibit invalidating spans on m_elements.
    MatchTree(const MatchTree& other) = delete;
    MatchTree& operator=(const MatchTree& other) = delete;
    MatchTree(MatchTree&& other) = delete;
    MatchTree& operator=(MatchTree&& other) = delete;

    void generate_applicable_elements_iteratively(const DenseState& state, std::vector<const Element*>& out_applicable_elements);

    size_t get_num_nodes() const;

    bool is_imperfect() const;
};

}

#endif
