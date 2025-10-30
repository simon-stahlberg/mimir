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

#ifndef MIMIR_SEARCH_FORMATTER_HPP_
#define MIMIR_SEARCH_FORMATTER_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{
namespace search
{
extern std::ostream& operator<<(std::ostream& os, const State& state);

extern std::ostream& operator<<(std::ostream& os, const PackedStateImpl& state);

extern std::ostream& operator<<(std::ostream& out, const Plan& element);

extern std::ostream& operator<<(std::ostream& out, const PartiallyOrderedPlan& element);

namespace match_tree
{
template<formalism::HasConjunctiveCondition E>
std::ostream& operator<<(std::ostream& out, const IInverseNode<E>& element);

template<formalism::HasConjunctiveCondition E>
std::ostream& operator<<(std::ostream& out, const INode<E>& element);
}  // end match_tree
}  // end search

extern std::ostream& print(std::ostream& os, const mimir::search::State& state);

extern std::ostream& print(std::ostream& os, const mimir::search::PackedStateImpl& state);

extern std::ostream& print(std::ostream& out, const mimir::search::Plan& element);

extern std::ostream& print(std::ostream& out, const mimir::search::PartiallyOrderedPlan& element);

template<mimir::formalism::HasConjunctiveCondition E>
std::ostream& print(std::ostream& out, const mimir::search::match_tree::IInverseNode<E>& element);

template<mimir::formalism::HasConjunctiveCondition E>
std::ostream& print(std::ostream& out, const mimir::search::match_tree::INode<E>& element);
}

#endif
