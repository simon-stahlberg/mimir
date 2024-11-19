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

/// @brief `ConstructorProperties` is a type trait to provide additional information related to dl constructors.
/// It is currently used only for generating composite constructors in the refinement process.
/// @tparam T
template<typename T>
struct ConstructorProperties
{
};

/**
 * Composite concepts
 */

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

template<>
struct ConstructorProperties<ConceptNegationImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Concept>;
};

template<>
struct ConstructorProperties<ConceptValueRestrictionImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Role, Concept>;
};

template<>
struct ConstructorProperties<ConceptExistentialQuantificationImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Role, Concept>;
};

template<>
struct ConstructorProperties<ConceptRoleValueMapContainmentImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Role, Role>;
};

template<>
struct ConstructorProperties<ConceptRoleValueMapEqualityImpl>
{
    using ConstructorType = Concept;
    using ArgumentTypes = std::tuple<Role, Role>;
};

/**
 * Composite roles
 */

template<>
struct ConstructorProperties<RoleIntersectionImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role, Role>;
};

template<>
struct ConstructorProperties<RoleUnionImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role, Role>;
};

template<>
struct ConstructorProperties<RoleComplementImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role>;
};

template<>
struct ConstructorProperties<RoleInverseImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role>;
};

template<>
struct ConstructorProperties<RoleCompositionImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role, Role>;
};

template<>
struct ConstructorProperties<RoleTransitiveClosureImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role>;
};

template<>
struct ConstructorProperties<RoleReflexiveTransitiveClosureImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role>;
};

template<>
struct ConstructorProperties<RoleRestrictionImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Role, Concept>;
};

template<>
struct ConstructorProperties<RoleIdentityImpl>
{
    using ConstructorType = Role;
    using ArgumentTypes = std::tuple<Concept>;
};

}

#endif
