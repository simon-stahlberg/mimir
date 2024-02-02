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


#ifndef MIMIR_FORMALISM_PROBLEM_GROUND_ATOM_HPP_
#define MIMIR_FORMALISM_PROBLEM_GROUND_ATOM_HPP_

#include "declarations.hpp"

#include <loki/problem/pddl/ground_atom.hpp>

#include <string>


namespace loki 
{
    template<typename HolderType, ElementsPerSegment N>
    class PersistentFactory;
}


namespace mimir 
{
    class GroundAtomImpl : public loki::Base<GroundAtomImpl> 
    {
    private:
        loki::pddl::GroundAtom external_;

        Predicate m_predicate;
        ObjectList m_objects;

        GroundAtomImpl(int identifier, loki::pddl::GroundAtom external, Predicate predicate, ObjectList objects);

        // Give access to the constructor.
        template<typename HolderType, ElementsPerSegment N>
        friend class loki::PersistentFactory;

        /// @brief Test for semantic equivalence
        bool is_structurally_equivalent_to_impl(const GroundAtomImpl& other) const;
        size_t hash_impl() const;
        void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

        // Give access to the private interface implementations.
        friend class loki::Base<GroundAtomImpl>;

    public:
        const Predicate& get_predicate() const;
        const ObjectList& get_objects() const;
    };
}


namespace std 
{
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<mimir::GroundAtom>
    {
        bool operator()(const mimir::GroundAtom& left_atom, const mimir::GroundAtom& right_atom) const;
    };

    template<>
    struct hash<mimir::GroundAtomImpl>
    {
        std::size_t operator()(const mimir::GroundAtomImpl& atom) const;
    };
}

#endif
