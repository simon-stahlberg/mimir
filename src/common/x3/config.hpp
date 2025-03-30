/*
 * Copyright (C) 2023 Dominik Drexler
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

#ifndef SRC_COMMON_X3_CONFIG_HPP_
#define SRC_COMMON_X3_CONFIG_HPP_

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/position_tagged.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>
#include <string>

namespace mimir::x3
{
namespace x3 = boost::spirit::x3;

using Position = x3::position_tagged;
using PositionList = std::vector<Position>;

// Our iterator type
typedef std::string::const_iterator iterator_type;

/* X3 error handler utility */
template<typename Iterator>
using error_handler = x3::error_handler<Iterator>;

using error_handler_tag = x3::error_handler_tag;

typedef error_handler<iterator_type> error_handler_type;

/* The phrase parse context */
typedef x3::phrase_parse_context<x3::ascii::space_type>::type phrase_context_type;

/* Combined error handler, pddl, and phrase parse Context */
typedef x3::context<error_handler_tag, std::reference_wrapper<error_handler_type>, phrase_context_type> context_type;
}

#endif
