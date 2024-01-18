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

#ifndef MIMIR_FORMALISM_DOMAIN_TERM_HPP_
#define MIMIR_FORMALISM_DOMAIN_TERM_HPP_

#include "declarations.hpp"

#include "../../common/mixins.hpp"

#include <loki/domain/pddl/term.hpp>

#include <string>


namespace loki {
template<typename HolderType, ElementsPerSegment N>
class PersistentFactory;
}



namespace mimir {

class TermObjectImpl : public loki::Base<TermObjectImpl> {
private:
    loki::pddl::Term external_;

    Object m_object;

    // Add additional members if needed.
    // Use the constructor to initialize them since they will not be needed to uniquely identify the object.
    // In this design, the compiler will automatically generate the memory layout.
    // We can optimize it by flattening it into a byte array and using this class as as a view
    // that reads offsets from the bytes and reinterprets bytes.

    TermObjectImpl(int identifier, loki::pddl::Term external, Object object);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const TermObjectImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<TermObjectImpl>;

public:
    const Object& get_object() const;
};


class TermVariableImpl : public loki::Base<TermVariableImpl> {
private:
    loki::pddl::Term external_;

    Variable m_variable;

    TermVariableImpl(int identifier, loki::pddl::Term external, Variable variable);

    // Give access to the constructor.
    template<typename HolderType, ElementsPerSegment N>
    friend class loki::PersistentFactory;

    bool is_structurally_equivalent_to_impl(const TermVariableImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<TermVariableImpl>;

public:
    const Variable& get_variable() const;
};

}



namespace std {
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<loki::pddl::Term>
    {
        bool operator()(const loki::pddl::Term& left_term, const loki::pddl::Term& right_term) const;
    };

    template<>
    struct hash<loki::pddl::TermObjectImpl>
    {
        std::size_t operator()(const loki::pddl::TermObjectImpl& term) const;
    };

    template<>
    struct hash<loki::pddl::TermVariableImpl>
    {
        std::size_t operator()(const loki::pddl::TermVariableImpl& term) const;
    };
}

#endif
