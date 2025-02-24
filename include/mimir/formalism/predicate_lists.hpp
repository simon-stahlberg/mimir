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

#ifndef MIMIR_FORMALISM_PREDICATE_LISTS_HPP_
#define MIMIR_FORMALISM_PREDICATE_LISTS_HPP_

#include "mimir/formalism/declarations.hpp"

#include <boost/hana.hpp>

namespace mimir
{
template<StaticOrFluentOrDerived... P>
class PredicateLists
{
private:
    boost::hana::map<boost::hana::pair<boost::hana::type<P>, PredicateList<P>>...> m_data;

public:
    template<StaticOrFluentOrDerived P_>
    PredicateList<P_>& get()
    {
        return boost::hana::at_key(m_data, boost::hana::type<P_> {});
    }

    template<StaticOrFluentOrDerived P_>
    const PredicateList<P_>& get() const
    {
        return boost::hana::at_key(m_data, boost::hana::type<P_> {});
    }
};
}

#endif
