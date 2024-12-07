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

#include "mimir/common/types_cista.hpp"
#include "mimir/formalism/declarations.hpp"

namespace mimir
{
class ProblemImpl
{
private:
    Index m_index;
    std::optional<fs::path> m_filepath;
    Domain m_domain;
    std::string m_name;
    Requirements m_requirements;
    ObjectList m_objects;
    PredicateList<Derived> m_derived_predicates;
    GroundLiteralList<Static> m_static_initial_literals;
    FlatBitset m_static_initial_positive_atoms;
    GroundLiteralList<Fluent> m_fluent_initial_literals;
    NumericFluentList m_numeric_fluents;
    GroundLiteralList<Static> m_static_goal_condition;
    GroundLiteralList<Fluent> m_fluent_goal_condition;
    GroundLiteralList<Derived> m_derived_goal_condition;
    std::optional<OptimizationMetric> m_optimization_metric;
    AxiomList m_axioms;

    // Below: add additional members if needed and initialize them in the constructor
    bool m_static_goal_holds;
    PredicateList<Derived> m_problem_and_domain_derived_predicates;
    AxiomList m_problem_and_domain_axioms;

    ProblemImpl(Index index,
                std::optional<fs::path> filepath,
                Domain domain,
                std::string name,
                Requirements requirements,
                ObjectList objects,
                PredicateList<Derived> derived_predicates,
                GroundLiteralList<Static> static_initial_literals,
                GroundLiteralList<Fluent> fluent_initial_literals,
                NumericFluentList numeric_fluents,
                GroundLiteralList<Static> static_goal_condition,
                GroundLiteralList<Fluent> fluent_goal_condition,
                GroundLiteralList<Derived> derived_goal_condition,
                std::optional<OptimizationMetric> optimization_metric,
                AxiomList axioms);

    // Give access to the constructor.
    template<typename HolderType, typename Hash, typename EqualTo>
    friend class loki::SegmentedRepository;

public:
    // moveable but not copyable
    ProblemImpl(const ProblemImpl& other) = delete;
    ProblemImpl& operator=(const ProblemImpl& other) = delete;
    ProblemImpl(ProblemImpl&& other) = default;
    ProblemImpl& operator=(ProblemImpl&& other) = default;

    Index get_index() const;
    const std::optional<fs::path>& get_filepath() const;
    const Domain& get_domain() const;
    const std::string& get_name() const;
    const Requirements& get_requirements() const;
    const ObjectList& get_objects() const;
    const PredicateList<Derived>& get_derived_predicates() const;
    const PredicateList<Derived>& get_problem_and_domain_derived_predicates() const;
    const GroundLiteralList<Static>& get_static_initial_literals() const;
    const FlatBitset& get_static_initial_positive_atoms_bitset() const;
    const GroundLiteralList<Fluent>& get_fluent_initial_literals() const;
    const NumericFluentList& get_numeric_fluents() const;
    template<PredicateTag P>
    const GroundLiteralList<P>& get_goal_condition() const;
    const std::optional<OptimizationMetric>& get_optimization_metric() const;
    const AxiomList& get_axioms() const;
    const AxiomList& get_problem_and_domain_axioms() const;
    bool static_goal_holds() const;
    bool static_literal_holds(const GroundLiteral<Static> literal) const;
};

extern std::ostream& operator<<(std::ostream& out, const ProblemImpl& element);

extern std::ostream& operator<<(std::ostream& out, Problem element);

}

#endif
