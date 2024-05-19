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

#include "mimir/common/collections.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/transformers/delete_relax.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators/dense_lifted.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/successor_state_generators/dense.hpp"
#include "mimir/search/translations.hpp"

#include <deque>
#include <memory>

namespace mimir
{

AAG<GroundedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) :
    AAG<GroundedAAGDispatcher<DenseStateTag>>(problem, pddl_factories, std::make_shared<DefaultGroundedAAGEventHandler>())
{
}

AAG<GroundedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories, std::shared_ptr<IGroundedAAGEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_factories(pddl_factories),
    m_event_handler(std::move(event_handler)),
    m_lifted_aag(m_problem, m_pddl_factories)
{
    // 1. Explore delete relaxed task. We explicitly require to keep actions and axioms with empty effects.
    auto delete_relax_transformer = DeleteRelaxTransformer(m_pddl_factories, false);
    const auto delete_free_problem = delete_relax_transformer.run(*m_problem);
    auto delete_free_lifted_aag = std::make_shared<LiftedDenseAAG>(delete_free_problem, m_pddl_factories);
    auto delete_free_ssg = DenseSSG(delete_free_lifted_aag);

    auto state_builder = StateBuilder();
    auto& state_atoms = state_builder.get_atoms_bitset();
    for (const auto& atom_id : delete_free_ssg.get_or_create_initial_state().get_atoms_bitset())
    {
        state_atoms.set(atom_id);
    }

    // Keep track of changes
    bool reached_delete_free_explore_fixpoint = true;

    auto actions = DenseGroundActionList {};
    do
    {
        reached_delete_free_explore_fixpoint = true;

        state_builder.get_flatmemory_builder().finish();
        const auto state = DenseState(FlatDenseState(state_builder.get_flatmemory_builder().buffer().data()));

        auto num_atoms_before = state_atoms.count();

        // Create and all applicable actions and apply them
        // Attention: we cannot just apply newly generated actions because conditional effects might trigger later.
        delete_free_lifted_aag->generate_applicable_actions(state, actions);
        for (const auto& action : actions)
        {
            const auto succ_state = delete_free_ssg.get_or_create_successor_state(state, action);
            for (const auto atom_id : succ_state.get_atoms_bitset())
            {
                state_atoms.set(atom_id);
            }
        }

        // Create and all applicable axioms and apply them
        delete_free_lifted_aag->generate_and_apply_axioms(state_atoms);

        auto num_atoms_after = state_atoms.count();

        if (num_atoms_before != num_atoms_after)
        {
            reached_delete_free_explore_fixpoint = false;
        }

    } while (!reached_delete_free_explore_fixpoint);

    m_event_handler->on_finish_delete_free_exploration(m_pddl_factories.get_ground_atoms_from_ids(state_atoms),
                                                       to_ground_actions(delete_free_lifted_aag->get_actions()),
                                                       to_ground_axioms(delete_free_lifted_aag->get_axioms()));

    // 2. Create ground actions
    auto ground_actions = DenseGroundActionList {};
    for (const auto& action : delete_free_lifted_aag->get_applicable_actions())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_action : delete_relax_transformer.get_unrelaxed_actions(action.get_action()))
        {
            auto action_arguments = ObjectList(action.get_objects().begin(), action.get_objects().end());
            auto grounded_action = m_lifted_aag.ground_action(unrelaxed_action, std::move(action_arguments));
            ground_actions.push_back(grounded_action);
        }
    }

    m_event_handler->on_finish_grounding_unrelaxed_actions(ground_actions);

    // 3. Build match tree
    m_action_match_tree = MatchTree(m_pddl_factories.get_ground_atoms_from_ids().size(), ground_actions);

    m_event_handler->on_finish_build_action_match_tree(m_action_match_tree);

    // 2. Create ground axioms
    auto ground_axioms = DenseGroundAxiomList {};
    for (const auto& axiom : delete_free_lifted_aag->get_applicable_axioms())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_axiom : delete_relax_transformer.get_unrelaxed_axioms(axiom.get_axiom()))
        {
            auto axiom_arguments = ObjectList(axiom.get_objects().begin(), axiom.get_objects().end());
            auto grounded_axiom = m_lifted_aag.ground_axiom(unrelaxed_axiom, std::move(axiom_arguments));
            ground_axioms.push_back(grounded_axiom);
        }
    }

    m_event_handler->on_finish_grounding_unrelaxed_axioms(ground_axioms);

    // 3. Build match tree
    m_axiom_match_tree = MatchTree(m_pddl_factories.get_ground_atoms_from_ids().size(), ground_axioms);

    m_event_handler->on_finish_build_axiom_match_tree(m_axiom_match_tree);
}

void AAG<GroundedAAGDispatcher<DenseStateTag>>::generate_applicable_actions_impl(DenseState state, DenseGroundActionList& out_applicable_actions)
{
    out_applicable_actions.clear();

    m_action_match_tree.get_applicable_elements(state.get_atoms_bitset(), out_applicable_actions);
}

void AAG<GroundedAAGDispatcher<DenseStateTag>>::generate_and_apply_axioms_impl(FlatBitsetBuilder& ref_state_atoms)
{
    auto tmp_builder = FlatBitsetBuilder();

    for (const auto& lifted_partition : m_lifted_aag.get_axiom_partitioning())
    {
        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms. */

            auto applicable_axioms = GroundAxiomList {};
            tmp_builder |= ref_state_atoms;
            tmp_builder.finish();
            auto state_bitset = FlatBitset(tmp_builder.buffer().data());
            m_axiom_match_tree.get_applicable_elements(state_bitset, applicable_axioms);

            /* Apply applicable axioms */

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(grounded_axiom.get_axiom());
                if (!lifted_partition.get_axioms().count(grounded_axiom.get_axiom()))
                {
                    // axiom not part of same partition
                    continue;
                }

                assert(grounded_axiom.is_applicable(ref_state_atoms));

                assert(!grounded_axiom.get_simple_effect().is_negated);

                const auto grounded_atom_id = grounded_axiom.get_simple_effect().atom_id;

                if (!ref_state_atoms.get(grounded_atom_id))
                {
                    // GENERATED NEW DERIVED ATOM!
                    reached_partition_fixed_point = false;
                }

                ref_state_atoms.set(grounded_atom_id);
            }

        } while (!reached_partition_fixed_point);
    }
}

void AAG<GroundedAAGDispatcher<DenseStateTag>>::on_finish_f_layer_impl() const { m_event_handler->on_finish_f_layer(); }

void AAG<GroundedAAGDispatcher<DenseStateTag>>::on_end_search_impl() const { m_event_handler->on_end_search(); }

[[nodiscard]] const FlatDenseActionSet& AAG<GroundedAAGDispatcher<DenseStateTag>>::get_actions() const { return m_lifted_aag.get_actions(); }

[[nodiscard]] ConstView<ActionDispatcher<DenseStateTag>> AAG<GroundedAAGDispatcher<DenseStateTag>>::get_action(size_t action_id) const
{
    return m_lifted_aag.get_action(action_id);
}

Problem AAG<GroundedAAGDispatcher<DenseStateTag>>::get_problem_impl() const { return m_problem; }

[[nodiscard]] PDDLFactories& AAG<GroundedAAGDispatcher<DenseStateTag>>::get_pddl_factories_impl() { return m_pddl_factories; }

[[nodiscard]] const PDDLFactories& AAG<GroundedAAGDispatcher<DenseStateTag>>::get_pddl_factories_impl() const { return m_pddl_factories; }

}
