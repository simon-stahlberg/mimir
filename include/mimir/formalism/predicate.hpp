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

#ifndef MIMIR_FORMALISM_PREDICATE_HPP_
#define MIMIR_FORMALISM_PREDICATE_HPP_

#include "declarations.hpp"

#include <loki/domain/pddl/predicate.hpp>
#include <string>

namespace loki
{
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}

namespace mimir
{
class PredicateImpl : public loki::Base<PredicateImpl>
{
private:
    std::string m_name;
    ParameterList m_parameters;

    // Below: add additional members if needed and initialize them in the constructor

    PredicateImpl(int identifier, std::string name, ParameterList parameters);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const PredicateImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<PredicateImpl>;

public:
    /// @brief Returns a parseable string representation in the context of a domain.
    void str(std::ostringstream& out, const loki::FormattingOptions& options, bool typing_enabled) const;

    const std::string& get_name() const;
    const ParameterList& get_parameters() const;
    size_t get_arity() const;
};
}

namespace std
{
// Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
template<>
struct less<mimir::Predicate>
{
    bool operator()(const mimir::Predicate& left_predicate, const mimir::Predicate& right_predicate) const;
};

template<>
struct hash<mimir::PredicateImpl>
{
    std::size_t operator()(const mimir::PredicateImpl& predicate) const;
};
}

#endif
