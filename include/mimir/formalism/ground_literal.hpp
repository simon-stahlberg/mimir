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

#ifndef MIMIR_FORMALISM_GROUND_LITERAL_HPP_
#define MIMIR_FORMALISM_GROUND_LITERAL_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/ground_literal.hpp>
#include <string>

namespace loki
{
template<typename HolderType>
class PersistentFactory;
}

namespace mimir
{
class GroundLiteralImpl : public loki::Base<GroundLiteralImpl>
{
private:
    bool m_is_negated;
    GroundAtom m_atom;

    // Below: add additional members if needed and initialize them in the constructor

    GroundLiteralImpl(int identifier, bool is_negated, GroundAtom atom);

    // Give access to the constructor.
    template<typename>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const GroundLiteralImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundLiteralImpl>;

public:
    bool is_negated() const;
    const GroundAtom& get_atom() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::GroundLiteral>
{
    bool operator()(const mimir::GroundLiteral& left_literal, const mimir::GroundLiteral& right_literal) const;
};

template<>
struct hash<mimir::GroundLiteralImpl>
{
    std::size_t operator()(const mimir::GroundLiteralImpl& literal) const;
};
}

#endif
