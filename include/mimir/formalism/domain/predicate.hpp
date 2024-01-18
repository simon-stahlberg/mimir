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

#ifndef MIMIR_FORMALISM_DOMAIN_PREDICATE_HPP_
#define MIMIR_FORMALISM_DOMAIN_PREDICATE_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/predicate.hpp>

#include <string>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {

// An alternative name is AtomSkeleton
class PredicateImpl : public loki::Base<PredicateImpl> {
private:
    loki::pddl::Predicate external_;

    std::string m_name;
    ParameterList m_parameters;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    PredicateImpl(int identifier, loki::pddl::Predicate external, std::string name, ParameterList parameters);

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
};

}


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::Predicate>
    {
        bool operator()(const loki::pddl::Predicate& left_predicate, const loki::pddl::Predicate& right_predicate) const;
    };

    template<>
    struct hash<loki::pddl::PredicateImpl>
    {
        std::size_t operator()(const loki::pddl::PredicateImpl& predicate) const;
    };
}

#endif
