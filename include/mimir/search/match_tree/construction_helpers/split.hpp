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

#ifndef MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_SPLIT_HPP_
#define MIMIR_SEARCH_MATCH_TREE_CONSTRUCTION_HELPERS_SPLIT_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/match_tree/declarations.hpp"

#include <concepts>
#include <variant>
#include <vector>

namespace mimir::match_tree
{

/**
 * Promising looking splits
 */

using CandidateSplit = std::variant<GroundAtom<Fluent>, GroundAtom<Derived>, GroundNumericConstraint>;
using CandidateSplitList = std::vector<CandidateSplit>;

/**
 * Element-separating splits with their separation and scoring functions
 */

inline double compute_gini_score(const std::vector<size_t>& class_distribution)
{
    size_t total = 0;
    for (size_t count : class_distribution)
    {
        total += count;
    }

    if (total == 0)
    {
        return 0.0;
    }

    double gini = 1.0;
    for (size_t count : class_distribution)
    {
        double probability = static_cast<double>(count) / total;
        gini -= probability * probability;
    }

    return gini;
}

template<DynamicPredicateTag P>
struct AtomSplit
{
    GroundAtom<P> feature;

    size_t m_num_true_elements;
    size_t m_num_false_elements;
    size_t m_num_dont_care_elements;

    bool is_useless() const
    {
        return (m_num_true_elements == 0 && m_num_false_elements == 0)         //
               || (m_num_true_elements == 0 && m_num_dont_care_elements == 0)  //
               || (m_num_false_elements == 0 && m_num_dont_care_elements == 0);
    }

    double compute_gini() const { return compute_gini_score(std::vector<size_t> { m_num_true_elements, m_num_false_elements, m_num_dont_care_elements }); }
};

struct NumericConstraintSplit
{
    GroundNumericConstraint feature;

    size_t m_num_true_elements;
    size_t m_num_dont_care_elements;

    bool is_useless() const
    {
        return (m_num_true_elements == 0)  //
               || (m_num_dont_care_elements == 0);
    }

    double compute_gini() const { return compute_gini_score(std::vector<size_t> { m_num_true_elements, m_num_dont_care_elements }); }
};

using Split = std::variant<AtomSplit<Fluent>, AtomSplit<Derived>, NumericConstraintSplit>;
using SplitList = std::vector<Split>;

}

#endif
