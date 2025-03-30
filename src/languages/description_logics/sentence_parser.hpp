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

#ifndef SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_SENTENCE_PARSER_HPP_
#define SRC_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTORS_SENTENCE_PARSER_HPP_

#include "mimir/formalism/domain.hpp"
#include "mimir/languages/description_logics/declarations.hpp"
#include "parser/ast.hpp"

#include <tuple>

namespace mimir::languages::dl
{

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
Constructor<D> parse_sentence(const std::string& sentence, const formalism::DomainImpl& domain, Repositories& repositories);

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
Constructor<D> parse(const ast::Constructor<D>& node, const formalism::DomainImpl& domain, Repositories& repositories);

}

#endif
