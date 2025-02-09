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

#ifndef MIMIR_SEARCH_MATCH_TREE_SPLIT_SCORING_FUNCTIONS_STATIC_FREQUENCY_HPP_
#define MIMIR_SEARCH_MATCH_TREE_SPLIT_SCORING_FUNCTIONS_STATIC_FREQUENCY_HPP_

#include "mimir/common/printers.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/atom.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/generator.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/interface.hpp"
#include "mimir/search/match_tree/construction_helpers/inverse_nodes/numeric_constraint.hpp"
#include "mimir/search/match_tree/split_scoring_functions/interface.hpp"

namespace mimir::match_tree
{
template<HasConjunctiveCondition Element>
class StaticFrequencySplitScoringFunction : public ISplitScoringFunction<Element>
{
private:
    const PDDLRepositories& m_pddl_repositories;
    CandidateSplitList m_splits;

    CandidateSplitList compute_static_frequency_splits(const std::vector<const Element*>& elements)
    {
        /* Collect frequencies */
        auto fluent_atom_frequencies = std::unordered_map<GroundAtom<Fluent>, size_t> {};
        auto derived_atom_frequencies = std::unordered_map<GroundAtom<Derived>, size_t> {};
        auto numeric_constraint_frequencies = std::unordered_map<GroundNumericConstraint, size_t> {};

        for (const auto& element : elements)
        {
            const auto& conjunctive_condition = element->get_conjunctive_condition();

            /* Fluent */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Fluent>())
            {
                ++fluent_atom_frequencies[m_pddl_repositories.template get_ground_atom<Fluent>(index)];
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Fluent>())
            {
                ++fluent_atom_frequencies[m_pddl_repositories.template get_ground_atom<Fluent>(index)];
            }
            /* Derived */
            for (const auto& index : conjunctive_condition.template get_positive_precondition<Derived>())
            {
                ++derived_atom_frequencies[m_pddl_repositories.template get_ground_atom<Derived>(index)];
            }
            for (const auto& index : conjunctive_condition.template get_negative_precondition<Derived>())
            {
                ++derived_atom_frequencies[m_pddl_repositories.template get_ground_atom<Derived>(index)];
            }
            /* Numeric constraint */
            for (const auto& numeric_constraint : conjunctive_condition.get_numeric_constraints())
            {
                ++numeric_constraint_frequencies[numeric_constraint.get()];
            }
        }

        std::cout << "fluent_atom_frequencies: " << fluent_atom_frequencies << std::endl;
        std::cout << "derived_atom_frequencies: " << derived_atom_frequencies << std::endl;
        std::cout << "numeric_constraint_frequencies: " << numeric_constraint_frequencies << std::endl;

        /* Sort splits descending in frequency, break ties in lexicographically*/
        std::vector<std::tuple<size_t, std::string, CandidateSplit>> sorted_splits;
        for (const auto& [atom, frequency] : fluent_atom_frequencies)
        {
            sorted_splits.emplace_back(frequency, to_string(atom), atom);
        }
        for (const auto& [atom, frequency] : derived_atom_frequencies)
        {
            sorted_splits.emplace_back(frequency, to_string(atom), atom);
        }
        for (const auto& [constraint, frequency] : numeric_constraint_frequencies)
        {
            sorted_splits.emplace_back(frequency, to_string(constraint), constraint);
        }
        std::sort(sorted_splits.begin(), sorted_splits.end(), std::greater());

        std::cout << "Sorted frequency splits: " << sorted_splits << std::endl;

        auto splits = CandidateSplitList {};
        for (const auto& [frequency, name, split] : sorted_splits)
        {
            splits.emplace_back(split);
        }

        std::cout << "Final splits: " << splits << std::endl;

        return splits;
    }

public:
    explicit StaticFrequencySplitScoringFunction(const PDDLRepositories& pddl_repositories, const std::vector<const Element*>& elements) :
        m_pddl_repositories(pddl_repositories),
        m_splits(compute_static_frequency_splits(elements))
    {
    }

    std::optional<SplitScoringFunctionResult> compute_best_split(std::span<const Element*> elements, const std::optional<InverseNode<Element>>& parent) override
    {
        // TODO: determine next non trivial split
    }
};

}

#endif
