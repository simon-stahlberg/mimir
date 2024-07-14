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

#include "mimir/formalism/predicate.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/variable.hpp"

namespace mimir
{

template<PredicateCategory P>
PredicateImpl<P>::PredicateImpl(int identifier, std::string name, VariableList parameters) :
    loki::Base<PredicateImpl<P>>(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
    assert(is_all_unique(m_parameters));
}

template<PredicateCategory P>
bool PredicateImpl<P>::is_structurally_equivalent_to_impl(const PredicateImpl<P>& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (m_parameters == other.m_parameters);
    }
    return true;
}

template<PredicateCategory P>
size_t PredicateImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_name, loki::hash_container(m_parameters));
}

template<PredicateCategory P>
void PredicateImpl<P>::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_name;
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        out << " ";
        m_parameters[i]->str(out, options);
    }
    out << ")";
}

template<PredicateCategory P>
const std::string& PredicateImpl<P>::get_name() const
{
    return m_name;
}

template<PredicateCategory P>
const VariableList& PredicateImpl<P>::get_parameters() const
{
    return m_parameters;
}

template<PredicateCategory P>
size_t PredicateImpl<P>::get_arity() const
{
    return m_parameters.size();
}

template class PredicateImpl<Static>;
template class PredicateImpl<Fluent>;
template class PredicateImpl<Derived>;

}
