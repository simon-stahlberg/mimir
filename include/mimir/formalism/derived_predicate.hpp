/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg and Simon Stahlberg
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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_DERIVED_PREDICATE_HPP_
#define MIMIR_FORMALISM_DERIVED_PREDICATE_HPP_

#include "mimir/formalism/declarations.hpp"

#include <loki/pddl/derived_predicate.hpp>
#include <loki/pddl/factory.hpp>
#include <string>

namespace mimir
{
class DerivedPredicateImpl : public loki::Base<DerivedPredicateImpl>
{
private:
    Predicate m_predicate;
    Condition m_condition;

    DerivedPredicateImpl(int identifier, Predicate predicate, Condition condition);

    // Give access to the constructor.
    friend class loki::PDDLFactory<DerivedPredicateImpl, loki::Hash<DerivedPredicateImpl*>, loki::EqualTo<DerivedPredicateImpl*>>;

    /// @brief Test for structural equivalence
    bool is_structurally_equivalent_to_impl(const DerivedPredicateImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<DerivedPredicateImpl>;

public:
    const Predicate& get_predicate() const;
    const Condition& get_condition() const;
};

}

#endif
