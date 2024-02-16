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

#ifndef MIMIR_FORMALISM_PROBLEM_PROBLEM_PDDL_PROBLEM_HPP_
#define MIMIR_FORMALISM_PROBLEM_PROBLEM_PDDL_PROBLEM_HPP_

#include "declarations.hpp"

#include <loki/problem/pddl/problem.hpp>

#include <string>
#include <optional>


namespace loki 
{
    template<typename HolderType, ElementsPerSegment N>
    class PersistentFactory;
}


namespace mimir 
{
    class ProblemImpl : public loki::Base<ProblemImpl> {
    private:
        Domain m_domain;
        std::string m_name;
        Requirements m_requirements;
        ObjectList m_objects;
        GroundLiteralList m_initial_literals;
        NumericFluentList m_numeric_fluents;
        Condition m_goal_condition;
        std::optional<OptimizationMetric> m_optimization_metric;

        // Below: add additional members if needed and initialize them in the constructor

        ProblemImpl(int identifier, Domain domain, std::string name, Requirements requirements, ObjectList objects, GroundLiteralList initial_literals, NumericFluentList numeric_fluents, Condition goal_condition, std::optional<OptimizationMetric> optimization_metric);

        // Give access to the constructor.
        template<typename HolderType, ElementsPerSegment N>
        friend class loki::PersistentFactory;

        /// @brief Test for semantic equivalence
        bool is_structurally_equivalent_to_impl(const ProblemImpl& other) const;
        size_t hash_impl() const;
        void str_impl(std::ostringstream& out, const loki::FormattingOptions& options) const;

        // Give access to the private interface implementations.
        friend class loki::Base<ProblemImpl>;

    public:
        const Domain& get_domain() const;
        const std::string& get_name() const;
        const Requirements& get_requirements() const;
        const ObjectList& get_objects() const;
        const GroundLiteralList& get_initial_literals() const;
        const NumericFluentList& get_numeric_fluents() const;
        const Condition& get_goal_condition() const;
        const std::optional<OptimizationMetric>& get_optimization_metric() const;
    };
}


namespace std 
{
    // Inject comparison and hash function to make pointers behave appropriately with ordered and unordered datastructures
    template<>
    struct less<mimir::Problem>
    {
        bool operator()(const mimir::Problem& left_problem, const mimir::Problem& right_problem) const;
    };

    template<>
    struct hash<mimir::ProblemImpl>
    {
        std::size_t operator()(const mimir::ProblemImpl& problem) const;
    };
}

#endif
