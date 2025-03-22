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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_DOMAIN_DETAILS_HPP_
#define MIMIR_FORMALISM_DOMAIN_DETAILS_HPP_

#include "mimir/formalism/declarations.hpp"

namespace mimir::formalism::domain
{

struct ConstantDetails
{
    const DomainImpl* parent;

    ToObjectMap<std::string> name_to_constant;

    ConstantDetails();
    ConstantDetails(const DomainImpl& domain);
};

struct PredicateDetails
{
    const DomainImpl* parent;

    ToPredicateMaps<std::string, StaticTag, FluentTag, DerivedTag> name_to_predicate;

    PredicateDetails();
    PredicateDetails(const DomainImpl& domain);
};

struct Details
{
    const DomainImpl* parent;

    ConstantDetails constant;
    PredicateDetails predicate;

    Details();
    Details(const DomainImpl& domain);
};
}

#endif
