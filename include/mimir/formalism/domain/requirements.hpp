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

#ifndef MIMIR_FORMALISM_DOMAIN_REQUIREMENTS_HPP_
#define MIMIR_FORMALISM_DOMAIN_REQUIREMENTS_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/requirements.hpp>

#include <string>
#include <set>
#include <unordered_map>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {

class RequirementsImpl : public loki::Base<RequirementsImpl> {
private:
    loki::pddl::Requirements external_;

    loki::pddl::RequirementEnumSet m_requirements;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    RequirementsImpl(int identifier, loki::pddl::Requirements external, loki::pddl::RequirementEnumSet requirements);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const RequirementsImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<RequirementsImpl>;

public:
    bool test(loki::pddl::RequirementEnum requirement) const;

    const loki::pddl::RequirementEnumSet& get_requirements() const;
};

}


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::Requirements>
    {
        bool operator()(const loki::pddl::Requirements& left_requirements, const loki::pddl::Requirements& right_requirements) const;
    };

    template<>
    struct hash<loki::pddl::RequirementsImpl>
    {
        std::size_t operator()(const loki::pddl::RequirementsImpl& requirements) const;
    };
}

#endif
