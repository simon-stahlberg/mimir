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

namespace mimir::dl
{

template<IsConceptOrRole D>
size_t DenotationRepository<D>::KeyHash::operator()(const Key& key) const
{
    return loki::hash_combine(key.constructor_identifier, key.state_identifier);
}

template<IsConceptOrRole D>
bool DenotationRepository<D>::KeyEqual::operator()(const Key& left, const Key& right) const
{
    if (&left != &right)
    {
        return (left.constructor_identifier == right.constructor_identifier) && (left.state_identifier == right.state_identifier);
    }
    return true;
}

template<IsConceptOrRole D>
Denotation<D> DenotationRepository<D>::insert(size_t constructor_identifier, size_t state_identifier, const DenotationBuilder<D>& denotation)
{
    const auto [it, inserted] = m_storage.insert(denotation.get_flatmemory_builder());
    if (inserted)
    {
        m_cached_dynamic_denotations.emplace(Key { constructor_identifier, state_identifier }, Denotation<D>(*it));
    }
    return Denotation<D>(*it);
}

template<IsConceptOrRole D>
std::optional<Denotation<D>> DenotationRepository<D>::get_if(size_t constructor_identifier, size_t state_identifier) const
{
    auto it = m_cached_dynamic_denotations.find(Key { constructor_identifier, state_identifier });
    if (it == m_cached_dynamic_denotations.end())
    {
        return std::nullopt;
    }
    return it->second;
}

template class DenotationRepository<Concept>;
template class DenotationRepository<Role>;

}
