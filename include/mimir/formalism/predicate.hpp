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

#ifndef MIMIR_FORMALISM_PREDICATE_HPP_
#define MIMIR_FORMALISM_PREDICATE_HPP_

#include "mimir/common/collections.hpp"
#include "mimir/formalism/predicate_category.hpp"
#include "mimir/formalism/variable.hpp"

#include <loki/loki.hpp>
#include <string>
#include <unordered_set>
#include <vector>

namespace mimir
{

/*
   TODO: Flattening PredicateImpl using a tuple with the following fields
   1) Flat indices
   - uint64_t m_identifier; (8 byte)
   - Vector<uint64_t> m_variable_ids; (variable sized)
   2) Data views
   - ConstView<String> m_name; (8 byte)
   - ConstView<Vector<Variable>> m_variables (8 byte)
*/
template<PredicateCategory P>
class PredicateImpl : public loki::Base<PredicateImpl<P>>
{
private:
    std::string m_name;
    VariableList m_parameters;

    // Below: add additional members if needed and initialize them in the constructor

    PredicateImpl(int identifier, std::string name, VariableList parameters);

    // Give access to the constructor.
    friend class loki::PDDLFactory<PredicateImpl<P>, loki::Hash<PredicateImpl<P>*>, loki::EqualTo<PredicateImpl<P>*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const PredicateImpl<P>& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<PredicateImpl<P>>;

public:
    using Category = P;

    const std::string& get_name() const;
    const VariableList& get_parameters() const;
    size_t get_arity() const;
};

/**
 * Implementation
 */

template<PredicateCategory P>
PredicateImpl<P>::PredicateImpl(int identifier, std::string name, VariableList parameters) :
    loki::Base<PredicateImpl<P>>(identifier),
    m_name(std::move(name)),
    m_parameters(std::move(parameters))
{
    assert(is_all_unique(m_parameters));
}

template<PredicateCategory P>
bool PredicateImpl<P>::is_structurally_equivalent_to_impl(const PredicateImpl<P>& other) const
{
    if (this != &other)
    {
        return (m_name == other.m_name) && (m_parameters == other.m_parameters);
    }
    return true;
}

template<PredicateCategory P>
size_t PredicateImpl<P>::hash_impl() const
{
    return loki::hash_combine(m_name, loki::hash_container(m_parameters));
}

template<PredicateCategory P>
void PredicateImpl<P>::str_impl(std::ostream& out, const loki::FormattingOptions& options) const
{
    out << "(" << m_name;
    for (size_t i = 0; i < m_parameters.size(); ++i)
    {
        out << " ";
        m_parameters[i]->str(out, options);
    }
    out << ")";
}

template<PredicateCategory P>
const std::string& PredicateImpl<P>::get_name() const
{
    return m_name;
}

template<PredicateCategory P>
const VariableList& PredicateImpl<P>::get_parameters() const
{
    return m_parameters;
}

template<PredicateCategory P>
size_t PredicateImpl<P>::get_arity() const
{
    return m_parameters.size();
}

/**
 * Type aliases
 */
template<PredicateCategory P>
using Predicate = const PredicateImpl<P>*;
template<PredicateCategory P>
using PredicateList = std::vector<Predicate<P>>;
template<PredicateCategory P>
using PredicateSet = std::unordered_set<Predicate<P>>;
template<typename Key, PredicateCategory P, typename Hash = std::hash<Key>, typename KeyEqual = std::equal_to<Key>>
using ToPredicateMap = std::unordered_map<Key, Predicate<P>, Hash, KeyEqual>;

}

#endif
