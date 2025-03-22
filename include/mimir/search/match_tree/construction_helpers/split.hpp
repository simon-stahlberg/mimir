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
#include <loki/loki.hpp>
#include <unordered_set>
#include <variant>
#include <vector>

namespace mimir::search::match_tree
{

/**
 * Promising looking splits
 */

using CandidateSplit =
    std::variant<formalism::GroundAtom<formalism::FluentTag>, formalism::GroundAtom<formalism::DerivedTag>, formalism::GroundNumericConstraint>;
using CandidateSplitList = std::vector<CandidateSplit>;

/**
 * Element-separating splits with their separation and scoring functions
 */

struct AtomSplitDistribution
{
    size_t num_true_elements = 0;
    size_t num_false_elements = 0;
    size_t num_dont_care_elements = 0;
};

template<formalism::IsFluentOrDerivedTag P>
struct AtomSplit
{
    formalism::GroundAtom<P> feature;

    AtomSplitDistribution distribution;

    auto identifying_members() const { return std::tuple(std::as_const(feature)); }
};

struct NumericConstraintSplitDistribution
{
    size_t num_true_elements;
    size_t num_dont_care_elements;
};

struct NumericConstraintSplit
{
    formalism::GroundNumericConstraint feature;
    NumericConstraintSplitDistribution distribution;

    auto identifying_members() const { return std::tuple(std::as_const(feature)); }
};

using Split = std::variant<AtomSplit<formalism::FluentTag>, AtomSplit<formalism::DerivedTag>, NumericConstraintSplit>;
using SplitList = std::vector<Split>;
using SplitSet = std::unordered_set<Split, loki::Hash<Split>, loki::EqualTo<Split>>;

struct SplitAndScore
{
    Split split;
    double score;
};

using SplitAndScoreList = std::vector<SplitAndScore>;

struct SplitScoreAndUselessSplits
{
    Split split;
    double score;
    SplitList useless_splits;
};

/**
 * Utils
 */

inline bool is_useless_split(const AtomSplitDistribution& distribution) { return distribution.num_true_elements == 0 && distribution.num_false_elements == 0; }

inline bool is_useless_split(const NumericConstraintSplitDistribution& distribution) { return distribution.num_true_elements == 0; }

template<formalism::IsFluentOrDerivedTag P>
bool is_useless_split(const AtomSplit<P>& split)
{
    return is_useless_split(split.distribution);
}

inline bool is_useless_split(const NumericConstraintSplit& split) { return is_useless_split(split.distribution); }

inline bool is_useless_split(const Split& split)
{
    return std::visit([](auto&& arg) { return is_useless_split(arg); }, split);
}

/**
 * Printing
 */

extern std::ostream& operator<<(std::ostream& out, const AtomSplitDistribution& distribution);

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintSplitDistribution& distribution);

template<formalism::IsFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const AtomSplit<P>& split);

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintSplit& split);

}

#endif
