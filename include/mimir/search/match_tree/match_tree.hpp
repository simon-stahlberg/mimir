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
#include "mimir/search/match_tree/node_splitters/interface.hpp"
#include "mimir/search/match_tree/nodes/interface.hpp"
#include "mimir/search/match_tree/options.hpp"
#include "mimir/search/match_tree/statistics.hpp"

namespace mimir::search::match_tree
{
/* MatchTree */
template<formalism::HasConjunctiveCondition E>
class MatchTreeImpl
{
private:
    std::vector<const E*> m_elements;  ///< ATTENTION: must remain persistent. Swapping elements is allowed.
    Options m_options;

    Node<E> m_root;
    Statistics m_statistics;

    std::vector<const INode<E>*> m_evaluate_stack;  ///< temporary during evaluation.

    MatchTreeImpl();

    MatchTreeImpl(const formalism::Repositories& pddl_repositories, std::vector<const E*> elements, const Options& options = Options());

public:
    static std::unique_ptr<MatchTreeImpl<E>>
    create(const formalism::Repositories& pddl_repositories, std::vector<const E*> elements, const Options& options = Options());

    // Uncopieable and unmoveable to prohibit invalidating spans on m_elements.
    MatchTreeImpl(const MatchTreeImpl& other) = delete;
    MatchTreeImpl& operator=(const MatchTreeImpl& other) = delete;
    MatchTreeImpl(MatchTreeImpl&& other) = delete;
    MatchTreeImpl& operator=(MatchTreeImpl&& other) = delete;

    void
    generate_applicable_elements_iteratively(const DenseState& state, const formalism::ProblemImpl& problem, std::vector<const E*>& out_applicable_elements);

    const Statistics& get_statistics() const;
};

}

#endif
