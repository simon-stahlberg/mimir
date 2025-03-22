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

#include "mimir/common/printers.hpp"
#include "mimir/formalism/ground_atom.hpp"
#include "mimir/formalism/ground_numeric_constraint.hpp"

using namespace mimir::formalism;

namespace mimir::search::match_tree
{

extern std::ostream& operator<<(std::ostream& out, const AtomSplitDistribution& distribution)
{
    out << "{ #true = " << distribution.num_true_elements    //
        << ", #false = " << distribution.num_false_elements  //
        << ", #dontcare = " << distribution.num_dont_care_elements << " }";
    return out;
}

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintSplitDistribution& distribution)
{
    out << "{ #true = " << distribution.num_true_elements  //
        << ", #dontcare = " << distribution.num_dont_care_elements << " }";
    return out;
}

template<IsFluentOrDerivedTag P>
std::ostream& operator<<(std::ostream& out, const AtomSplit<P>& split)
{
    out << "[" << split.feature << " -> " << split.distribution << "]";
    return out;
}

template std::ostream& operator<<(std::ostream& out, const AtomSplit<FluentTag>& split);
template std::ostream& operator<<(std::ostream& out, const AtomSplit<DerivedTag>& split);

extern std::ostream& operator<<(std::ostream& out, const NumericConstraintSplit& split)
{
    out << "[" << split.feature << " -> " << split.distribution << "]";
    return out;
}

}
