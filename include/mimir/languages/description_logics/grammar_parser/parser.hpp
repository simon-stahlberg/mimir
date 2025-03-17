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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_GRAMMAR_PARSER_PARSER_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_GRAMMAR_PARSER_PARSER_HPP_

#include "mimir/languages/description_logics/parser/ast.hpp"

#include <boost/spirit/home/x3.hpp>

namespace mimir::languages::dl
{
namespace x3 = boost::spirit::x3;

///////////////////////////////////////////////////////////////////////////
// parser public interface
///////////////////////////////////////////////////////////////////////////
namespace grammar_parser
{
struct GrammarClass;

typedef x3::rule<GrammarClass, ast::Grammar> grammar_type;

BOOST_SPIRIT_DECLARE(grammar_type)
}

grammar_parser::grammar_type const& grammar_parser_();

}

#endif
