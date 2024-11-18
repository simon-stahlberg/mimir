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

#ifndef MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_PROPERTIES_HPP_
#define MIMIR_LANGUAGES_DESCRIPTION_LOGICS_CONSTRUCTOR_PROPERTIES_HPP_

#include "mimir/languages/description_logics/constructor_tag.hpp"
#include "mimir/languages/description_logics/constructors.hpp"

namespace mimir::dl
{
template<typename T>
struct ConstructorProperties
{
};

template<>
struct ConstructorProperties<ConceptIntersectionImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Concept, Concept>;
};

template<>
struct ConstructorProperties<ConceptUnionImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Concept, Concept>;
};
}

#endif
