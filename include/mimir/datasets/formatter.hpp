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

#ifndef MIMIR_DATASETS_FORMATTER_HPP_
#define MIMIR_DATASETS_FORMATTER_HPP_

#include "mimir/datasets/declarations.hpp"

#include <ostream>

namespace mimir
{
namespace graphs
{
extern std::ostream& operator<<(std::ostream& out, const ProblemVertex& element);

extern std::ostream& operator<<(std::ostream& out, const ProblemEdge& element);

extern std::ostream& operator<<(std::ostream& out, const ClassVertex& element);

extern std::ostream& operator<<(std::ostream& out, const ClassEdge& element);
}  // end graphs

namespace datasets
{
extern std::ostream& operator<<(std::ostream& out, const TupleGraphImpl& element);
}  // end datasets

extern std::ostream& print(std::ostream& out, const mimir::graphs::ProblemVertex& element);

extern std::ostream& print(std::ostream& out, const mimir::graphs::ProblemEdge& element);

extern std::ostream& print(std::ostream& out, const mimir::graphs::ClassVertex& element);

extern std::ostream& print(std::ostream& out, const mimir::graphs::ClassEdge& element);

extern std::ostream& print(std::ostream& out, const mimir::datasets::TupleGraphImpl& element);
}

#endif
