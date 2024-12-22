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

#include "mimir/search/grounders/axiom_grounder.hpp"

#include "mimir/common/memory.hpp"
#include "mimir/formalism/object.hpp"
#include "mimir/formalism/repositories.hpp"
#include "mimir/search/grounders/literal_grounder.hpp"

namespace mimir
{
AxiomGrounder::AxiomGrounder(std::shared_ptr<LiteralGrounder> literal_grounder) :
    m_literal_grounder(std::move(literal_grounder)),
    m_axioms(),
    m_axioms_by_index(),
    m_per_axiom_data()
{
}

/// @brief Ground an axiom and return a view onto it.
GroundAxiom AxiomGrounder::ground_axiom(Axiom axiom, ObjectList binding)
{
    /* 1. Check if grounding is cached */

    auto& [axiom_builder, grounding_table] = m_per_axiom_data[axiom];
    auto it = grounding_table.find(binding);
    if (it != grounding_table.end())
    {
        return it->second;
    }

    /* 2. Ground the axiom */

    /* Header */

    axiom_builder.get_index() = m_axioms.size();
    axiom_builder.get_axiom() = axiom->get_index();
    auto& objects = axiom_builder.get_object_indices();
    objects.clear();
    for (const auto& obj : binding)
    {
        objects.push_back(obj->get_index());
    }
    objects.compress();

    /* Precondition */
    auto& strips_precondition = axiom_builder.get_strips_precondition();
    auto& positive_fluent_precondition = strips_precondition.get_positive_precondition<Fluent>();
    auto& negative_fluent_precondition = strips_precondition.get_negative_precondition<Fluent>();
    auto& positive_static_precondition = strips_precondition.get_positive_precondition<Static>();
    auto& negative_static_precondition = strips_precondition.get_negative_precondition<Static>();
    auto& positive_derived_precondition = strips_precondition.get_positive_precondition<Derived>();
    auto& negative_derived_precondition = strips_precondition.get_negative_precondition<Derived>();
    positive_fluent_precondition.clear();
    negative_fluent_precondition.clear();
    positive_static_precondition.clear();
    negative_static_precondition.clear();
    positive_derived_precondition.clear();
    negative_derived_precondition.clear();
    m_literal_grounder->ground_and_fill_vector(axiom->get_precondition()->get_literals<Fluent>(),
                                               positive_fluent_precondition,
                                               negative_fluent_precondition,
                                               binding);
    m_literal_grounder->ground_and_fill_vector(axiom->get_precondition()->get_literals<Static>(),
                                               positive_static_precondition,
                                               negative_static_precondition,
                                               binding);
    m_literal_grounder->ground_and_fill_vector(axiom->get_precondition()->get_literals<Derived>(),
                                               positive_derived_precondition,
                                               negative_derived_precondition,
                                               binding);
    positive_fluent_precondition.compress();
    negative_fluent_precondition.compress();
    positive_static_precondition.compress();
    negative_static_precondition.compress();
    positive_derived_precondition.compress();
    negative_derived_precondition.compress();

    /* Effect */

    // The effect literal might only use the first few objects of the complete binding
    // Therefore, we can prevent the literal grounding table from unnecessarily growing
    // by restricting the binding to only the relevant part
    const auto effect_literal_arity = axiom->get_literal()->get_atom()->get_arity();
    const auto is_complete_binding_relevant_for_head = (binding.size() == effect_literal_arity);
    const auto grounded_literal =
        is_complete_binding_relevant_for_head ?
            m_literal_grounder->ground_literal(axiom->get_literal(), binding) :
            m_literal_grounder->ground_literal(axiom->get_literal(), ObjectList(binding.begin(), binding.begin() + effect_literal_arity));
    assert(!grounded_literal->is_negated());
    axiom_builder.get_derived_effect().is_negated = false;
    axiom_builder.get_derived_effect().atom_index = grounded_literal->get_atom()->get_index();

    const auto [iter, inserted] = m_axioms.insert(axiom_builder);
    const auto grounded_axiom = iter->get();

    if (inserted)
    {
        m_axioms_by_index.push_back(grounded_axiom);
    }

    /* 3. Insert to groundings table */

    grounding_table.emplace(std::move(binding), GroundAxiom(grounded_axiom));

    /* 4. Return the resulting ground axiom */

    return grounded_axiom;
}

/// @brief Return all axioms.
const GroundAxiomList& AxiomGrounder::get_ground_axioms() const { return m_axioms_by_index; }

GroundAxiom AxiomGrounder::get_ground_axiom(Index axiom_index) const { return m_axioms_by_index.at(axiom_index); }

Problem AxiomGrounder::get_problem() const { return m_literal_grounder->get_problem(); }

const std::shared_ptr<PDDLRepositories>& AxiomGrounder::get_pddl_repositories() const { return m_literal_grounder->get_pddl_repositories(); }

const std::shared_ptr<LiteralGrounder>& AxiomGrounder::get_literal_grounder() const { return m_literal_grounder; }

size_t AxiomGrounder::get_num_ground_axioms() const { return m_axioms_by_index.size(); }

size_t AxiomGrounder::get_estimated_memory_usage_in_bytes_for_axioms() const
{
    const auto usage1 = m_axioms.get_estimated_memory_usage_in_bytes();
    const auto usage2 = m_axioms_by_index.capacity() * sizeof(GroundAxiom);
    auto usage3 = size_t(0);
    // TODO: add memory usage of m_per_axiom_data
    for (const auto& [axiom, action_data] : m_per_axiom_data)
    {
        const auto& [axiom_builder, grounding_table] = action_data;
        // TODO: add memory usage of axiom_builder
        usage3 += get_memory_usage_in_bytes(grounding_table);
    }

    return usage1 + usage2 + usage3;
}
}
