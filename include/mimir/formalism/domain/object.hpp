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

#ifndef MIMIR_FORMALISM_DOMAIN_OBJECT_HPP_
#define MIMIR_FORMALISM_DOMAIN_OBJECT_HPP_

#include "declarations.hpp"

#include "../common/base.hpp"

#include <loki/domain/pddl/object.hpp>

#include <string>
#include <functional>


namespace loki {
template<typename HolderType, mimir::ElementsPerSegment N>
class PersistentFactory;
}


namespace mimir {
class ObjectImpl : public Base<ObjectImpl> {
private:
    loki::pddl::Object external_;

    ObjectImpl(int identifier, loki::pddl::Object external_);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const ObjectImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class Base<ObjectImpl>;

public:
    const std::string& get_name() const;
    const TypeList& get_bases() const;
};

}  // namespace mimir


namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<mimir::Object>
    {
        bool operator()(const mimir::Object& left_object, const mimir::Object& right_object) const;
    };

    template<>
    struct hash<mimir::ObjectImpl>
    {
        std::size_t operator()(const mimir::ObjectImpl& object) const;
    };
}  // namespace std


#endif  // MIMIR_FORMALISM_DOMAIN_OBJECT_HPP_
