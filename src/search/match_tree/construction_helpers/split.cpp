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

#include "mimir/search/match_tree/construction_helpers/split.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/formalism/formatter.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"

#include <fmt/core.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

std::ostream& operator<<(std::ostream& out, const AtomSplitDistribution& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const NumericConstraintSplitDistribution& element) { return mimir::print(out, element); }

template<IsFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const AtomSplit<P>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const AtomSplit<FluentTag>& split);
template std::ostream& operator<<(std::ostream& out, const AtomSplit<DerivedTag>& split);

std::ostream& operator<<(std::ostream& out, const NumericConstraintSplit& element) { return mimir::print(out, element); }

}

namespace mimir
{
std::ostream& print(std::ostream& out, const mimir::search::match_tree::AtomSplitDistribution& element)
{
    fmt::print(out, "< #true =  {}, #false = {}, #dontcare = {} >", element.num_true_elements, element.num_false_elements, element.num_dont_care_elements);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::match_tree::NumericConstraintSplitDistribution& element)
{
    fmt::print(out, "< #true =  {}, #dontcare = {} >", element.num_true_elements, element.num_dont_care_elements);
    return out;
}

template<mimir::formalism::IsFluentOrDerivedTag P>
std::ostream& print(std::ostream& out, const mimir::search::match_tree::AtomSplit<P>& element)
{
    fmt::print(out, "[{} -> {}]", to_string(element.feature), to_string(element.distribution));
    return out;
}

std::ostream& print(std::ostream& out, const mimir::search::match_tree::NumericConstraintSplit& element)
{
    fmt::print(out, "[{} -> {}]", to_string(element.feature), to_string(element.distribution));
    return out;
}
}