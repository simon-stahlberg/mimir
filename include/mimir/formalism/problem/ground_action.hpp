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

#ifndef MIMIR_FORMALISM_PROBLEM_GROUND_ACTION_HPP_
#define MIMIR_FORMALISM_PROBLEM_GROUND_ACTION_HPP_

#include "declarations.hpp"

#include <string>

namespace loki
{
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}

namespace mimir
{
class GroundActionImpl : public loki::Base<GroundActionImpl>
{
private:
    Action m_action;
    ObjectList m_terms;
    LiteralList m_condition;
    LiteralList m_effect;

    GroundActionImpl(int identifier, Action action, ObjectList objects, LiteralList condition, LiteralList effect);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const GroundActionImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<GroundActionImpl>;

public:
    const Action& get_action() const;
    const ObjectList& get_terms() const;
    const LiteralList& get_condition() const;
    const LiteralList& get_effect() const;
    size_t get_arity() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::GroundAction>
{
    bool operator()(const mimir::GroundAction& left_action, const mimir::GroundAction& right_action) const;
};

template<>
struct hash<mimir::GroundActionImpl>
{
    std::size_t operator()(const mimir::GroundActionImpl& action) const;
};
}

#endif
