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

#include "mimir/languages/description_logics/denotation_repositories.hpp"

using namespace mimir::formalism;

namespace mimir::languages::dl
{

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
Denotation<D> DenotationRepository<D>::insert(Constructor<D> constructor, search::State state, const DenotationImpl<D>& denotation)
{
    const auto [it, inserted] = m_storage.insert(denotation);

    if (inserted)
    {
        m_cached_dynamic_denotations.emplace(Key { constructor, state }, Denotation<D>(it->get()));
    }
    return Denotation<D>(it->get());
}

template<IsConceptOrRoleOrBooleanOrNumericalTag D>
Denotation<D> DenotationRepository<D>::get_if(Constructor<D> constructor, search::State state) const
{
    auto it = m_cached_dynamic_denotations.find(Key { constructor, state });

    if (it == m_cached_dynamic_denotations.end())
    {
        return nullptr;
    }
    return it->second;
}

template class DenotationRepository<ConceptTag>;
template class DenotationRepository<RoleTag>;
template class DenotationRepository<BooleanTag>;
template class DenotationRepository<NumericalTag>;

}
