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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_HPP_

#include "mimir/algorithms/generator.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/consistency_graph.hpp"
#include "mimir/search/declarations.hpp"

namespace mimir
{

class SatisficingBindingGenerator
{
private:
    std::shared_ptr<LiteralGrounder> m_literal_grounder;

    VariableList m_variables;
    LiteralList<Static> m_static_conditions;
    LiteralList<Fluent> m_fluent_conditions;
    LiteralList<Derived> m_derived_conditions;

    std::shared_ptr<ISatisficingBindingGeneratorEventHandler> m_event_handler;

    /* Precomputed grounded conditions with zero arguments. */
    GroundLiteralList<Static> m_nullary_static_conditions;
    GroundLiteralList<Fluent> m_nullary_fluent_conditions;
    GroundLiteralList<Derived> m_nullary_derived_conditions;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    template<DynamicPredicateTag P>
    bool is_valid_dynamic_binding(const LiteralList<P>& literals, State state, const ObjectList& binding);

    bool is_valid_static_binding(const LiteralList<Static>& literals, const ObjectList& binding);

    bool is_valid_binding(State state, const ObjectList& binding);

    /// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
    bool nullary_conditions_hold(State state) const;

    mimir::generator<ObjectList> nullary_case(State state);

    mimir::generator<ObjectList>
    unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets, const AssignmentSet<Derived>& derived_assignment_sets, State state);

    mimir::generator<ObjectList>
    general_case(const AssignmentSet<Fluent>& fluent_assignment_sets, const AssignmentSet<Derived>& derived_assignment_sets, State state);

public:
    SatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                VariableList variables,
                                LiteralList<Static> static_conditions,
                                LiteralList<Fluent> fluent_conditions,
                                LiteralList<Derived> derived_conditions);

    SatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                VariableList variables,
                                LiteralList<Static> static_conditions,
                                LiteralList<Fluent> fluent_conditions,
                                LiteralList<Derived> derived_conditions,
                                std::shared_ptr<ISatisficingBindingGeneratorEventHandler> event_handler);

    mimir::generator<ObjectList>
    create_binding_generator(State state, const AssignmentSet<Fluent>& fluent_assignment_set, const AssignmentSet<Derived>& derived_assignment_set);

    mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
    create_ground_conjunction_generator(State state);

    /**
     * Getters
     */

    const std::shared_ptr<LiteralGrounder>& get_literal_grounder() const;

    const VariableList& get_variables() const;
    template<PredicateTag P>
    const LiteralList<P>& get_conditions() const;
    const std::shared_ptr<ISatisficingBindingGeneratorEventHandler>& get_event_handler() const;
    const consistency_graph::StaticConsistencyGraph& get_static_consistency_graph() const;
};

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const SatisficingBindingGenerator& condition_grounder);

}

#endif
