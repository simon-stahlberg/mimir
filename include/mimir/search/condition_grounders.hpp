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

#include "mimir/common/printers.hpp"
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
    std::shared_ptr<PDDLFactories> m_pddl_factories;
    std::shared_ptr<IConditionGrounderEventHandler> m_event_handler;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    template<DynamicPredicateCategory P>
    bool is_valid_dynamic_binding(const LiteralList<P>& literals, const State state, const ObjectList& binding);

    bool is_valid_static_binding(const Problem problem, const LiteralList<Static>& literals, const ObjectList& binding);

    bool is_valid_binding(const Problem problem, const State state, const ObjectList& binding);

    template<PredicateCategory P>
    bool nullary_literals_hold(const LiteralList<P>& literals, const Problem problem, const State state, PDDLFactories& pddl_factories);

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_conditions_hold(const Problem problem, const State state);

    void nullary_case(const State state, std::vector<ObjectList>& ref_bindings);

    void unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                    const AssignmentSet<Derived>& derived_assignment_sets,
                    const State state,
                    std::vector<ObjectList>& ref_bindings);

    void general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                      const AssignmentSet<Derived>& derived_assignment_sets,
                      const State state,
                      std::vector<ObjectList>& ref_bindings);

public:
    ConditionGrounder(Problem problem,
                      VariableList variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      AssignmentSet<Static> static_assignment_set,
                      std::shared_ptr<PDDLFactories> pddl_factories);

    ConditionGrounder(Problem problem,
                      VariableList variables,
                      LiteralList<Static> static_conditions,
                      LiteralList<Fluent> fluent_conditions,
                      LiteralList<Derived> derived_conditions,
                      AssignmentSet<Static> static_assignment_set,
                      std::shared_ptr<PDDLFactories> pddl_factories,
                      std::shared_ptr<IConditionGrounderEventHandler> event_handler);

    void compute_bindings(const State state,
                          const AssignmentSet<Fluent>& fluent_assignment_set,
                          const AssignmentSet<Derived>& derived_assignment_set,
                          std::vector<ObjectList>& out_bindings);

    /**
     * Getters
     */

    Problem get_problem() const;
};

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const ConditionGrounder& condition_grounder);

}

#endif
