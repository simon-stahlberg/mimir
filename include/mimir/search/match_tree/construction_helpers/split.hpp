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

struct AtomSplitDistribution
{
    size_t m_num_true_elements = 0;
    size_t m_num_false_elements = 0;
    size_t m_num_dont_care_elements = 0;
};

template<DynamicPredicateTag P>
struct AtomSplit
{
    GroundAtom<P> feature;

    AtomSplitDistribution m_distribution;
};

struct NumericConstraintSplitDistribution
{
    size_t m_num_true_elements;
    size_t m_num_dont_care_elements;
};

struct NumericConstraintSplit
{
    GroundNumericConstraint feature;
    NumericConstraintSplitDistribution m_distribution;
};

using Split = std::variant<AtomSplit<Fluent>, AtomSplit<Derived>, NumericConstraintSplit>;
using SplitList = std::vector<Split>;

/**
 * Utils
 */

inline bool is_useless_split(const AtomSplitDistribution& distribution)
{
    return distribution.m_num_true_elements == 0 && distribution.m_num_false_elements == 0;
}

inline bool is_useless_split(const NumericConstraintSplitDistribution& distribution) { return distribution.m_num_true_elements == 0; }

template<DynamicPredicateTag P>
bool is_useless_split(const AtomSplit<P>& split)
{
    return is_useless_split(split.m_distribution);
}

inline bool is_useless_split(const NumericConstraintSplit& split) { return is_useless_split(split.m_distribution); }

inline bool is_useless_split(const Split& split)
{
    return std::visit([](auto&& arg) { return is_useless_split(arg); }, split);
}

/**
 * Printing
 */

extern std::ostream& operator<<(std::ostream& out, const AtomSplitDistribution& distribution);

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintSplitDistribution& distribution);

template<DynamicPredicateTag P>
std::ostream& operator<<(std::ostream& out, const AtomSplit<P>& split);

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintSplit& split);

}

#endif
