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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_ACTION_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_ACTION_HPP_

#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/interface.hpp"

namespace mimir
{

class ActionSatisficingBindingGenerator : public SatisficingBindingGenerator<ActionSatisficingBindingGenerator>
{
private:
    /* Implement SatisficingBindingGenerator interface. */
    friend class SatisficingBindingGenerator<ActionSatisficingBindingGenerator>;

    bool is_valid_binding_impl(const DenseState& dense_state, const ObjectList& binding);

    using SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::is_valid_binding;

private:
    /* Declare additional private members and functions. */
    Action m_action;

    template<DynamicFunctionTag F>
    bool is_valid_binding(NumericEffect<F> effect, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding);

    template<DynamicFunctionTag F>
    bool is_valid_binding(const NumericEffectList<F>& effects, const FlatDoubleList& fluent_numeric_variables, const ObjectList& binding);

    bool is_valid_binding(ConjunctiveEffect effect, const DenseState& dense_state, const ObjectList& binding);

    bool is_valid_binding_if_fires(ConditionalEffect effect, const DenseState& dense_state, const ObjectList& binding);

public:
    using SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::create_binding_generator;
    using SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::create_ground_conjunction_generator;
    using SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::get_event_handler;
    using SatisficingBindingGenerator<ActionSatisficingBindingGenerator>::get_static_consistency_graph;

    ActionSatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                      std::shared_ptr<NumericConstraintGrounder> numeric_constraint_grounder,
                                      Action action,
                                      std::optional<std::shared_ptr<ISatisficingBindingGeneratorEventHandler>> event_handler = std::nullopt);
};

}

#endif
