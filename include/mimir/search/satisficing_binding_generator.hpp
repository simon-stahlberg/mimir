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
#include "mimir/algorithms/kpkc.hpp"
#include "mimir/formalism/declarations.hpp"
#include "mimir/search/consistency_graph.hpp"
#include "mimir/search/declarations.hpp"
#include "mimir/search/workspaces.hpp"

#include <boost/dynamic_bitset/dynamic_bitset.hpp>
#include <memory_resource>

namespace mimir
{

/**
 * SatisficingBindingGeneratorWorkspace
 */

class SatisficingBindingGeneratorWorkspace
{
private:
    friend class SatisficingBindingGenerator;

    std::vector<boost::dynamic_bitset<>>& get_or_create_full_consistency_graph(const consistency_graph::StaticConsistencyGraph& static_consistency_graph);
    KPKCWorkspace& get_or_create_kpkc_workspace(const consistency_graph::StaticConsistencyGraph& static_consistency_graph);
    AssignmentSetWorkspace& get_or_create_assignment_set_workspace();

    std::optional<std::vector<boost::dynamic_bitset<>>> full_consistency_graph;
    std::optional<KPKCWorkspace> kpkc_workspace;
    std::optional<AssignmentSetWorkspace> assignment_set_workspace;
};

/**
 * Utils
 */

/// @brief Returns true if all nullary literals in the precondition hold, false otherwise.
extern bool nullary_conditions_hold(ExistentiallyQuantifiedConjunctiveCondition precondition, State state);

/**
 * SatisficingBindingGenerator
 */

class SatisficingBindingGenerator
{
private:
    std::shared_ptr<LiteralGrounder> m_literal_grounder;
    ExistentiallyQuantifiedConjunctiveCondition m_precondition;
    std::shared_ptr<ISatisficingBindingGeneratorEventHandler> m_event_handler;

    consistency_graph::StaticConsistencyGraph m_static_consistency_graph;

    template<DynamicPredicateTag P>
    bool is_valid_dynamic_binding(const LiteralList<P>& literals, State state, const ObjectList& binding);

    bool is_valid_static_binding(const LiteralList<Static>& literals, const ObjectList& binding);

    bool is_valid_binding(State state, const ObjectList& binding);

    mimir::generator<ObjectList> nullary_case(State state);

    mimir::generator<ObjectList>
    unary_case(const AssignmentSet<Fluent>& fluent_assignment_sets, const AssignmentSet<Derived>& derived_assignment_sets, State state);

    mimir::generator<ObjectList> general_case(const AssignmentSet<Fluent>& fluent_assignment_sets,
                                              const AssignmentSet<Derived>& derived_assignment_sets,
                                              State state,
                                              SatisficingBindingGeneratorWorkspace& workspace);

public:
    SatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder, ExistentiallyQuantifiedConjunctiveCondition precondition);

    SatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                ExistentiallyQuantifiedConjunctiveCondition precondition,
                                std::shared_ptr<ISatisficingBindingGeneratorEventHandler> event_handler);

    mimir::generator<ObjectList> create_binding_generator(State state,
                                                          const AssignmentSet<Fluent>& fluent_assignment_set,
                                                          const AssignmentSet<Derived>& derived_assignment_set,
                                                          SatisficingBindingGeneratorWorkspace& workspace);

    mimir::generator<std::pair<ObjectList, std::tuple<GroundLiteralList<Static>, GroundLiteralList<Fluent>, GroundLiteralList<Derived>>>>
    create_ground_conjunction_generator(State state, SatisficingBindingGeneratorWorkspace& workspace);

    /**
     * Getters
     */

    const std::shared_ptr<LiteralGrounder>& get_literal_grounder() const;

    const ExistentiallyQuantifiedConjunctiveCondition& get_precondition() const;
    const std::shared_ptr<ISatisficingBindingGeneratorEventHandler>& get_event_handler() const;
    const consistency_graph::StaticConsistencyGraph& get_static_consistency_graph() const;
};

/**
 * Pretty printing
 */

std::ostream& operator<<(std::ostream& out, const SatisficingBindingGenerator& condition_grounder);

}

#endif
