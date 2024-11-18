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

#ifndef MIMIR_SEARCH_CONDITION_GROUNDERS_HPP_
#define MIMIR_SEARCH_CONDITION_GROUNDERS_HPP_

#include "mimir/formalism/declarations.hpp"
#include "mimir/search/applicable_action_generators/lifted/assignment_set.hpp"
#include "mimir/search/applicable_action_generators/lifted/consistency_graph.hpp"
#include "mimir/search/condition_grounders/event_handlers/interface.hpp"
#include "mimir/search/state.hpp"

#include <memory>
#include <ostream>
#include <stdexcept>
#include <tuple>
#include <utility>

namespace mimir
{

class ConditionGrounder
{
private:
    Problem m_problem;
    VariableList m_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;
    AssignmentSet<Static> m_static_assignment_set;
    std::shared_ptr<PDDLRepositories> m_pddl_repositories;
    std::shared_ptr<IConditionGrounderEventHandler> m_event_handler;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    template<DynamicPredicateTag P>
    bool is_valid_dynamic_binding(const LiteralList<P>& literals, State state, const ObjectList& binding);

    bool is_valid_static_binding(Problem problem, const LiteralList<Static>& literals, const ObjectList& binding);

    bool is_valid_binding(Problem problem, State state, const ObjectList& binding);

    template<DynamicPredicateTag P>
    bool nullary_literals_hold(const LiteralList<P>& literals, Problem problem, State state, PDDLRepositories& pddl_repositories);

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_conditions_hold(Problem problem, State state);

    void nullary_case(State state, std::vector<ObjectList>& ref_bindings);

    void unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                    const AssignmentSet<Derived>& derived_assignment_sets,
                    State state,
                    std::vector<ObjectList>& ref_bindings);

    void general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                      const AssignmentSet<Derived>& derived_assignment_sets,
                      State state,
                      std::vector<ObjectList>& ref_bindings);

public:
    ConditionGrounder(Problem problem,
                      VariableList variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      AssignmentSet<Static> static_assignment_set,
                      std::shared_ptr<PDDLRepositories> pddl_repositories);

    ConditionGrounder(Problem problem,
                      VariableList variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      AssignmentSet<Static> static_assignment_set,
                      std::shared_ptr<PDDLRepositories> pddl_repositories,
                      std::shared_ptr<IConditionGrounderEventHandler> event_handler);

    void compute_bindings(State state,
                          const AssignmentSet<Fluent>& fluent_assignment_set,
                          const AssignmentSet<Derived>& derived_assignment_set,
                          std::vector<ObjectList>& out_bindings);

    /**
     * Getters
     */

    Problem get_problem() const;
    const VariableList& get_variables() const;
    template<PredicateTag P>
    const LiteralList<P>& get_conditions() const;
    const AssignmentSet<Static>& get_static_assignment_set() const;
    const std::shared_ptr<PDDLRepositories>& get_pddl_repositories() const;
    const std::shared_ptr<IConditionGrounderEventHandler>& get_event_handler() const;
    const consistency_graph::StaticConsistencyGraph& get_static_consistency_graph() const;
};

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const ConditionGrounder& condition_grounder);

}

#endif
