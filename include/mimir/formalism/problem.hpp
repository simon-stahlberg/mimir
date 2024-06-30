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

#ifndef MIMIR_FORMALISM_PROBLEM_HPP_
#define MIMIR_FORMALISM_PROBLEM_HPP_

#include "mimir/common/concepts.hpp"
#include "mimir/formalism/axiom.hpp"
#include "mimir/formalism/domain.hpp"
#include "mimir/formalism/ground_literal.hpp"
#include "mimir/formalism/metric.hpp"
#include "mimir/formalism/numeric_fluent.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/requirements.hpp"
#include "mimir/search/flat_types.hpp"

#include <loki/loki.hpp>
#include <optional>
#include <string>
#include <vector>

namespace mimir
{
class ProblemImpl : public loki::Base<ProblemImpl>
{
private:
    Domain m_domain;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    PredicateList<Derived> m_derived_predicates;
    PredicateList<Derived> m_problem_and_domain_derived_predicates;
    GroundLiteralList<Static> m_static_initial_literals;
    FlatBitsetBuilder<Static> m_static_initial_positive_atoms_builder;
    GroundLiteralList<Fluent> m_fluent_initial_literals;
    NumericFluentList m_numeric_fluents;
    GroundLiteralList<Static> m_static_goal_condition;
    GroundLiteralList<Fluent> m_fluent_goal_condition;
    GroundLiteralList<Derived> m_derived_goal_condition;
    std::optional<OptimizationMetric> m_optimization_metric;
    AxiomList m_axioms;

    // Below: add additional members if needed and initialize them in the constructor

    bool m_static_goal_holds;

    ProblemImpl(int identifier,
                Domain domain,
                std::string name,
                Requirements requirements,
                ObjectList objects,
                PredicateList<Derived> derived_predicates,
                PredicateList<Derived> problem_and_domain_derived_predicates,
                GroundLiteralList<Static> static_initial_literals,
                GroundLiteralList<Fluent> fluent_initial_literals,
                NumericFluentList numeric_fluents,
                GroundLiteralList<Static> static_goal_condition,
                GroundLiteralList<Fluent> fluent_goal_condition,
                GroundLiteralList<Derived> derived_goal_condition,
                std::optional<OptimizationMetric> optimization_metric,
                AxiomList axioms);

    // Give access to the constructor.
    friend class loki::PDDLFactory<ProblemImpl, loki::Hash<ProblemImpl*>, loki::EqualTo<ProblemImpl*>>;

    /// @brief Test for semantic equivalence
    bool is_structurally_equivalent_to_impl(const ProblemImpl& other) const;
    size_t hash_impl() const;
    void str_impl(std::ostream& out, const loki::FormattingOptions& options) const;

    // Give access to the private interface implementations.
    friend class loki::Base<ProblemImpl>;

public:
    const Domain& get_domain() const;
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_objects() const;
    const PredicateList<Derived>& get_derived_predicates() const;
    const PredicateList<Derived>& get_problem_and_domain_derived_predicates() const;
    const GroundLiteralList<Static>& get_static_initial_literals() const;
    FlatBitset<Static> get_static_initial_positive_atoms_bitset() const;
    const GroundLiteralList<Fluent>& get_fluent_initial_literals() const;
    const NumericFluentList& get_numeric_fluents() const;
    template<PredicateCategory P>
    const GroundLiteralList<P>& get_goal_condition() const;
    const std::optional<OptimizationMetric>& get_optimization_metric() const;
    const AxiomList& get_axioms() const;
    bool static_goal_holds() const;
    bool static_literal_holds(const GroundLiteral<Static> literal) const;
};

/**
 * Type aliases
 */

using Problem = const ProblemImpl*;
using ProblemList = std::vector<Problem>;

/**
 * Implementations
 */

template<PredicateCategory P>
const GroundLiteralList<P>& ProblemImpl::get_goal_condition() const
{
    if constexpr (std::is_same_v<P, Static>)
    {
        return m_static_goal_condition;
    }
    else if constexpr (std::is_same_v<P, Fluent>)
    {
        return m_fluent_goal_condition;
    }
    else if constexpr (std::is_same_v<P, Derived>)
    {
        return m_derived_goal_condition;
    }
    else
    {
        static_assert(dependent_false<P>::value, "Missing implementation for PredicateCategory.");
    }
}

}

#endif
