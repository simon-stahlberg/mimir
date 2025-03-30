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

#ifndef SRC_LANGUAGES_GENERAL_POLICIES_PARSER_HPP_
#define SRC_LANGUAGES_GENERAL_POLICIES_PARSER_HPP_

#include "mimir/formalism/domain.hpp"
#include "mimir/languages/general_policies/declarations.hpp"

namespace mimir::languages::general_policies
{
extern GeneralPolicy parse(const std::string& description, const formalism::DomainImpl& domain, Repositories& repositories, dl::Repositories& dl_repositories);
}

#endif
