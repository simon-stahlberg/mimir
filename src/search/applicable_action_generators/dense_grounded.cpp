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
 *<
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/applicable_action_generators/dense_grounded.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/formalism/transformers/delete_relax.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/event_handlers/minimal.hpp"
#include "mimir/search/successor_state_generators/dense.hpp"

#include <deque>

namespace mimir
{

AAG<GroundedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) :
    m_problem(problem),
    m_pddl_factories(pddl_factories),
    m_lifted_aag(m_problem, m_pddl_factories)
{
    // 1. Explore delete relaxed task.
    auto delete_relax_transformer = DeleteRelaxTransformer(m_pddl_factories);
    const auto dr_problem = delete_relax_transformer.run(*m_problem);
    auto dr_lifted_aag = std::make_shared<LiftedDenseAAG>(dr_problem, m_pddl_factories);
    auto dr_ssg = DenseSSG(dr_problem, dr_lifted_aag);

    auto& state_bitset = m_state_builder.get_atoms_bitset();
    state_bitset.unset_all();
    for (const auto& atom_id : dr_ssg.get_or_create_initial_state(m_problem).get_atoms_bitset())
    {
        state_bitset.set(atom_id);
    }

    // Keep track of changes
    size_t num_atoms = 0;
    size_t num_actions = 0;
    // Temporary variables
    auto actions = DenseActionList {};
    auto state_atoms = FlatBitsetBuilder();
    auto derived_atoms = FlatBitsetBuilder();
    do
    {
        num_atoms = m_pddl_factories.get_ground_atoms().size();
        num_actions = dr_lifted_aag->get_actions().size();

        m_state_builder.get_flatmemory_builder().finish();
        const auto state = DenseState(FlatDenseState(m_state_builder.get_flatmemory_builder().buffer().data()));

        // Create and all applicable actions and apply them
        dr_lifted_aag->generate_applicable_actions(state, actions);
        for (const auto& action : actions)
        {
            const auto is_newly_generated = (action.get_id() >= num_actions);
            if (is_newly_generated)
            {
                const auto succ_state = dr_ssg.get_or_create_successor_state(state, action);
                for (const auto atom_id : succ_state.get_atoms_bitset())
                {
                    state_bitset.set(atom_id);
                }
            }
        }

        // Create and all applicable axioms and apply them
        state_atoms |= state_bitset;
        dr_lifted_aag->generate_and_apply_axioms(state_atoms, derived_atoms);

    } while (num_atoms != m_pddl_factories.get_ground_atoms().size());

    std::cout << "Total number of ground atoms reachable in delete-relaxed task: " << m_pddl_factories.get_ground_atoms().size() << std::endl;
    std::cout << "Total number of ground actions in delete-relaxed task: " << dr_lifted_aag->get_actions().size() << std::endl;

    // 2. Create ground actions
    auto ground_actions = DenseActionList {};
    for (const auto& action : dr_lifted_aag->get_actions())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        const auto action_proxy = DenseAction(action);
        const auto& unrelaxed_action = delete_relax_transformer.get_unrelaxed_action(action_proxy.get_action());
        auto action_arguments = ObjectList(action_proxy.get_objects().begin(), action_proxy.get_objects().end());
        ground_actions.push_back(m_lifted_aag.ground_action(unrelaxed_action, std::move(action_arguments)));
    }

    // 3. Build match tree
    m_action_match_tree = MatchTree(m_pddl_factories.get_ground_atoms().size(), ground_actions);

    // 2. Create ground axioms
    auto ground_axioms = DenseAxiomList {};
    for (const auto& axiom : dr_lifted_aag->get_axioms())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        const auto axiom_proxy = DenseAxiom(axiom);
        const auto& unrelaxed_axiom = delete_relax_transformer.get_unrelaxed_axiom(axiom_proxy.get_axiom());
        auto axiom_arguments = ObjectList(axiom_proxy.get_objects().begin(), axiom_proxy.get_objects().end());
        ground_axioms.push_back(m_lifted_aag.ground_axiom(unrelaxed_axiom, std::move(axiom_arguments)));
    }

    // 3. Build match tree
    m_axiom_match_tree = MatchTree(m_pddl_factories.get_ground_atoms().size(), ground_axioms);

    std::cout << "Total number of ground actions in task: " << ground_actions.size() << std::endl;
    std::cout << "Total number of ground axioms in task: " << ground_axioms.size() << std::endl;
    std::cout << "Total number of nodes in match tree: " << m_action_match_tree.get_num_nodes() << std::endl;
}

void AAG<GroundedAAGDispatcher<DenseStateTag>>::generate_applicable_actions_impl(DenseState state, DenseActionList& out_applicable_actions)
{
    out_applicable_actions.clear();

    m_action_match_tree.get_applicable_elements(state.get_atoms_bitset(), out_applicable_actions);
}

void AAG<GroundedAAGDispatcher<DenseStateTag>>::generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_state_atoms, FlatBitsetBuilder& ref_derived_atoms)
{
    // In the grounded case, we traverse a match tree, apply axioms, and repeat until fixed point.
    // We have to partition axioms first

    /* 1. Compute applicable axioms. */
    auto applicable_axioms = GroundAxiomList {};
    ref_state_atoms.finish();
    auto state_bitset = FlatBitset(ref_state_atoms.buffer().data());
    m_axiom_match_tree.get_applicable_elements(state_bitset, applicable_axioms);

    /* 2. Group ground axioms by lifted axiom */
    auto ground_axioms_by_lifted_axiom = std::unordered_map<Axiom, GroundAxiomList> {};
    for (const auto& ground_axiom : applicable_axioms)
    {
        ground_axioms_by_lifted_axiom[ground_axiom.get_axiom()].push_back(ground_axiom);
    }

    /* 3. Partition ground axioms. */
    for (const auto& lifted_partition : m_lifted_aag.get_axiom_partitioning())
    {
        auto grounded_axiom_partition = GroundAxiomList {};
        for (const auto& lifted_axiom : lifted_partition.get_axioms())
        {
            const auto it = ground_axioms_by_lifted_axiom.find(lifted_axiom);
            if (it != ground_axioms_by_lifted_axiom.end())
            {
                const auto& grounded_axioms = it->second;
                grounded_axiom_partition.insert(grounded_axiom_partition.end(), grounded_axioms.begin(), grounded_axioms.end());
            }
        }

        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            for (const auto& grounded_axiom : grounded_axiom_partition)
            {
                if (grounded_axiom.is_applicable(ref_state_atoms))
                {
                    const auto grounded_atom_id = grounded_axiom.get_simple_effect();

                    if (!ref_state_atoms.get(grounded_atom_id))
                    {
                        // GENERATED NEW DERIVED ATOM!
                        reached_partition_fixed_point = false;
                    }

                    ref_state_atoms.set(grounded_atom_id);
                    ref_derived_atoms.set(grounded_atom_id);
                }
            }

        } while (!reached_partition_fixed_point);
    }
}

[[nodiscard]] const FlatDenseActionSet& AAG<GroundedAAGDispatcher<DenseStateTag>>::get_actions() const { return m_lifted_aag.get_actions(); }

[[nodiscard]] ConstView<ActionDispatcher<DenseStateTag>> AAG<GroundedAAGDispatcher<DenseStateTag>>::get_action(size_t action_id) const
{
    return m_lifted_aag.get_action(action_id);
}
}
