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

#include "mimir/common/hash.hpp"

namespace mimir::dl
{

template<ConstructorTag D>
size_t DenotationRepository<D>::KeyHash::operator()(const Key& key) const
{
    return mimir::hash_combine(key.constructor, key.state_index);
}

template<ConstructorTag D>
bool DenotationRepository<D>::KeyEqual::operator()(const Key& left, const Key& right) const
{
    if (&left != &right)
    {
        return (left.constructor == right.constructor) && (left.state_index == right.state_index);
    }
    return true;
}

template<ConstructorTag D>
Denotation<D> DenotationRepository<D>::insert(Constructor<D> constructor, size_t state_index, const DenotationImpl<D>& denotation)
{
    const auto [it, inserted] = m_storage.insert(denotation);
    if (inserted)
    {
        m_cached_dynamic_denotations.emplace(Key { constructor, state_index }, Denotation<D>(*it));
    }
    return Denotation<D>(*it);
}

template<ConstructorTag D>
std::optional<Denotation<D>> DenotationRepository<D>::get_if(Constructor<D> constructor, size_t state_index) const
{
    auto it = m_cached_dynamic_denotations.find(Key { constructor, state_index });
    if (it == m_cached_dynamic_denotations.end())
    {
        return std::nullopt;
    }
    return it->second;
}

template class DenotationRepository<Concept>;
template class DenotationRepository<Role>;

}
