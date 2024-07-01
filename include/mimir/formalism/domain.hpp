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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef MIMIR_FORMALISM_DOMAIN_HPP_
#define MIMIR_FORMALISM_DOMAIN_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/action.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/function_skeleton.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"

#include <loki/loki.hpp>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <vector>

namespace mimir
{
class DomainImpl : public loki::Base<DomainImpl>
{
private:
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_constants;
    PredicateList<Static> m_static_predicates;
    PredicateList<Fluent> m_fluent_predicates;
    PredicateList<Derived> m_derived_predicates;
    FunctionSkeletonList m_functions;
    ActionList m_actions;
    AxiomList m_axioms;

    // Below: add additional members if needed and initialize them in the constructor
    ToPredicateMap<std::string, Static> m_name_to_static_predicate;
    ToPredicateMap<std::string, Fluent> m_name_to_fluent_predicate;
    ToPredicateMap<std::string, Derived> m_name_to_derived_predicate;

    DomainImpl(int identifier,
               std::string name,
               Requirements requirements,
               ObjectList constants,
               PredicateList<Static> static_predicates,
               PredicateList<Fluent> fluent_predicates,
               PredicateList<Derived> derived_predicates,
               FunctionSkeletonList functions,
               ActionList actions,
               AxiomList axioms);

    // Give access to the constructor.
    friend class loki::PDDLFactory<DomainImpl, loki::Hash<DomainImpl*>, loki::EqualTo<DomainImpl*>>;

    /// @brief Test for structural equivalence
    bool is_structurally_equivalent_to_impl(const DomainImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<DomainImpl>;

public:
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_constants() const;
    template<PredicateCategory P>
    const PredicateList<P>& get_predicates() const;
    const FunctionSkeletonList& get_functions() const;
    const ActionList& get_actions() const;
    const AxiomList& get_axioms() const;

    template<PredicateCategory P>
    const ToPredicateMap<std::string, P>& get_name_to_predicate() const;
};

/**
 * Type alises
 */

using Domain = const DomainImpl*;
using DomainList = std::vector<Domain>;

/**
 * Implementations
 */

template<PredicateCategory P>
const PredicateList<P>& DomainImpl::get_predicates() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_predicates;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_predicates;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_predicates;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

template<PredicateCategory P>
const ToPredicateMap<std::string, P>& DomainImpl::get_name_to_predicate() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_name_to_static_predicate;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_name_to_fluent_predicate;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_name_to_derived_predicate;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}
}

#endif
