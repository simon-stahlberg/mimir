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

#ifndef MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_AXIOM_HPP_
#define MIMIR_SEARCH_SATISFICING_BINDING_GENERATOR_AXIOM_HPP_

#include "mimir/search/declarations.hpp"
#include "mimir/search/satisficing_binding_generators/interface.hpp"

namespace mimir
{

class AxiomSatisficingBindingGenerator : public SatisficingBindingGenerator<AxiomSatisficingBindingGenerator>
{
private:
    /* Implement SatisficingBindingGenerator interface. */
    friend class SatisficingBindingGenerator<AxiomSatisficingBindingGenerator>;

    Axiom m_axiom;

public:
    using SatisficingBindingGenerator<AxiomSatisficingBindingGenerator>::create_binding_generator;
    using SatisficingBindingGenerator<AxiomSatisficingBindingGenerator>::create_ground_conjunction_generator;
    using SatisficingBindingGenerator<AxiomSatisficingBindingGenerator>::get_event_handler;
    using SatisficingBindingGenerator<AxiomSatisficingBindingGenerator>::get_static_consistency_graph;

    AxiomSatisficingBindingGenerator(std::shared_ptr<LiteralGrounder> literal_grounder,
                                     std::shared_ptr<NumericConstraintGrounder> numeric_constraint_grounder,
                                     Axiom axiom,
                                     std::optional<std::shared_ptr<ISatisficingBindingGeneratorEventHandler>> event_handler = std::nullopt);
};

}

#endif
