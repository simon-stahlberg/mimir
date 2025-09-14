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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_CONJUNCTIVE_CONDITION_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_CONJUNCTIVE_CONDITION_HPP_

#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/base.hpp"

namespace mimir::search
{

class ConjunctiveConditionSatisficingBindingGenerator : public SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>
{
private:
    /* Implement SatisficingBindingGenerator interface. */
    friend class SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>;

    bool is_valid_binding_impl(const UnpackedStateImpl& unpacked_state, const formalism::ObjectList& binding) { return true; }

public:
    using SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>::create_binding_generator;
    using SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>::create_ground_conjunction_generator;
    using SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>::get_event_handler;
    using SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>::get_static_consistency_graph;

    ConjunctiveConditionSatisficingBindingGenerator(formalism::ConjunctiveCondition conjunctive_condition,
                                                    formalism::Problem problem,
                                                    EventHandler event_handler = nullptr);
};

}

#endif