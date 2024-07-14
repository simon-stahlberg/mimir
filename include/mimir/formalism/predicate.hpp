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

#include "mimir/formalism/declarations.hpp"

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

}

#endif
