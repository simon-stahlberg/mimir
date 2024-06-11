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

#ifndef MIMIR_FORMALISM_GROUND_ATOM_HPP_
#define MIMIR_FORMALISM_GROUND_ATOM_HPP_

#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"

#include <loki/loki.hpp>
#include <ostream>
#include <string>

namespace mimir
{
/*
   TODO: Flattening GroundAtomImpl using a tuple with the following fields:
   1) Flat indices
   - uint64_t m_identifier; (8 byte)
   - uint64_t m_predicate_id; (8 byte)
   - Vector<uint64_t> m_object_ids; (variable bytes)
   2) Data views
   - ConstView<Predicate> m_predicate; (8 byte)
   - ConstView<Vector<Object>> m_objects; (8 byte)
*/
template<PredicateCategory P>
class GroundAtomImpl : public loki::Base<GroundAtomImpl<P>>
{
private:
    Predicate<P> m_predicate;
    ObjectList m_objects;

    // Below: add additional members if needed and initialize them in the constructor

    GroundAtomImpl(int identifier, Predicate<P> predicate, ObjectList objects);

    // Give access to the constructor.
    friend class loki::PDDLFactory<GroundAtomImpl, loki::Hash<GroundAtomImpl*>, loki::EqualTo<GroundAtomImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const GroundAtomImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundAtomImpl>;

public:
    using Category = P;

    Predicate<P> get_predicate() const;
    const ObjectList& get_objects() const;
    bool is_static() const;
    size_t get_arity() const;
};

/**
 * Type aliases
 */

template<PredicateCategory P>
using GroundAtom = const GroundAtomImpl<P>*;
template<PredicateCategory P>
using GroundAtomList = std::vector<GroundAtom<P>>;
template<PredicateCategory P>
using GroundAtomSet = std::unordered_set<GroundAtom<P>>;

/**
 * Implementation details
 */

template<PredicateCategory P>
GroundAtomImpl<P>::GroundAtomImpl(int identifier, Predicate<P> predicate, ObjectList objects) :
    loki::Base<GroundAtomImpl<P>>(identifier),
    m_predicate(std::move(predicate)),
    m_objects(std::move(objects))
{
}

template<PredicateCategory P>
bool GroundAtomImpl<P>::is_structurally_equivalent_to_impl(const GroundAtomImpl<P>& other) const
{
    if (this != &other)
    {
        return (m_predicate == other.m_predicate) && (m_objects == other.m_objects);
    }
    return true;
}

template<PredicateCategory P>
size_t GroundAtomImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_predicate, loki::hash_container(m_objects));
}

template<PredicateCategory P>
void GroundAtomImpl<P>::str_impl(std::ostream& out, const loki::FormattingOptions& /*options*/) const
{
    out << "(" << m_predicate->get_name();
    for (size_t i = 0; i < m_objects.size(); ++i)
    {
        out << " " << m_objects[i]->get_name();
    }
    out << ")";
}

template<PredicateCategory P>
Predicate<P> GroundAtomImpl<P>::get_predicate() const
{
    return m_predicate;
}

template<PredicateCategory P>
const ObjectList& GroundAtomImpl<P>::get_objects() const
{
    return m_objects;
}

template<PredicateCategory P>
bool GroundAtomImpl<P>::is_static() const
{
    return m_predicate->is_static();
}

template<PredicateCategory P>
size_t GroundAtomImpl<P>::get_arity() const
{
    return m_objects.size();
}
}

#endif
