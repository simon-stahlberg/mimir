/*
 * Copyright (C) 2023 Dominik Drexler
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

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/factory.hpp>
#include <loki/pddl/ground_atom.hpp>
#include <string>

namespace mimir
{
class GroundAtomImpl : public loki::Base<GroundAtomImpl>
{
private:
    Predicate m_predicate;
    ObjectList m_objects;

    // Below: add additional members if needed and initialize them in the constructor

    GroundAtomImpl(int identifier, Predicate predicate, ObjectList objects);

    // Give access to the constructor.
    template<typename>
    friend class loki::PDDLFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const GroundAtomImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundAtomImpl>;

public:
    const Predicate& get_predicate() const;
    const ObjectList& get_objects() const;
    size_t get_arity() const;
};
}

#endif
