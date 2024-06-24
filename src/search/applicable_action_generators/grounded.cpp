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

#include "mimir/search/applicable_action_generators/grounded.hpp"

#include "mimir/common/collections.hpp"
#include "mimir/common/itertools.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/search/algorithms/brfs.hpp"
#include "mimir/search/applicable_action_generators/interface.hpp"
#include "mimir/search/applicable_action_generators/lifted.hpp"
#include "mimir/search/axiom_evaluators/axiom_stratification.hpp"
#include "mimir/search/successor_state_generator.hpp"
#include "mimir/search/translations.hpp"

#include <deque>
#include <memory>

namespace mimir
{

/// @brief Compute a ground atom order where ground atoms over same predicate are next to each other
/// The basic idea is that those can be potential mutex variables and grouping them together
/// can result in a smaller match tree. Such math tree structures have size linear in the number of mutex variables.
/// We also consider larger groups first since such mutex variables would result in a very large linear split.
template<PredicateCategory P>
static std::vector<size_t> compute_ground_atom_order(const GroundAtomList<P>& atoms, const PDDLFactories& pddl_factories)
{
    auto ground_atoms_order = std::vector<size_t> {};
    auto m_ground_atoms_by_predicate = std::unordered_map<Predicate<P>, GroundAtomList<P>> {};
    for (const auto& ground_atom : atoms)
    {
        m_ground_atoms_by_predicate[ground_atom->get_predicate()].push_back(ground_atom);
    }
    auto ground_atoms = GroundAtomList<P> {};
    // Sort group decreasingly in their size, break ties lexicographically by predicate name
    auto sorted_groups = std::vector<std::pair<Predicate<P>, GroundAtomList<P>>> {};
    for (const auto& [predicate, group] : m_ground_atoms_by_predicate)
    {
        sorted_groups.emplace_back(predicate, group);
    }
    std::sort(sorted_groups.begin(),
              sorted_groups.end(),
              [](const std::pair<Predicate<P>, GroundAtomList<P>>& left, const std::pair<Predicate<P>, GroundAtomList<P>>& right)
              {
                  if (left.second.size() == right.second.size())
                  {
                      return left.first->get_name() < right.first->get_name();
                  }
                  return left.second.size() > right.second.size();
              });
    for (const auto& [_predicate, group] : sorted_groups)
    {
        // Sort grounded atoms in each group by indices to get compiler independent results.
        auto sorted_group = group;
        std::sort(sorted_group.begin(), sorted_group.end(), [](const GroundAtom<P>& left, const GroundAtom<P>& right) { return left->str() < right->str(); });
        for (const auto& grounded_atom : sorted_group)

        {
            ground_atoms_order.push_back(grounded_atom->get_identifier());
        }
    }
    return ground_atoms_order;
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(Problem problem, std::shared_ptr<PDDLFactories> pddl_factories) :
    GroundedApplicableActionGenerator(problem, std::move(pddl_factories), std::make_shared<DefaultGroundedAAGEventHandler>())
{
}

GroundedApplicableActionGenerator::GroundedApplicableActionGenerator(Problem problem,
                                                                     std::shared_ptr<PDDLFactories> pddl_factories,
                                                                     std::shared_ptr<IGroundedAAGEventHandler> event_handler) :
    m_problem(problem),
    m_pddl_factories(std::move(pddl_factories)),
    m_event_handler(std::move(event_handler)),
    m_lifted_aag(m_problem, m_pddl_factories)
{
    /* 1. Explore delete relaxed task. We explicitly require to keep actions and axioms with empty effects. */
    auto delete_relax_transformer = DeleteRelaxTransformer(*m_pddl_factories, false);
    const auto delete_free_problem = delete_relax_transformer.run(*m_problem);
    auto delete_free_lifted_aag = std::make_shared<LiftedAAG>(delete_free_problem, m_pddl_factories);
    auto delete_free_ssg = SuccessorStateGenerator(delete_free_lifted_aag);

    auto state_builder = StateBuilder();
    auto& fluent_state_atoms = state_builder.get_atoms<Fluent>();
    auto& derived_state_atoms = state_builder.get_atoms<Derived>();
    fluent_state_atoms = delete_free_ssg.get_or_create_initial_state().get_atoms<Fluent>();

    // Keep track of changes
    bool reached_delete_free_explore_fixpoint = true;

    auto actions = GroundActionList {};
    do
    {
        reached_delete_free_explore_fixpoint = true;

        state_builder.get_flatmemory_builder().finish();
        const auto state = State(FlatState(state_builder.get_flatmemory_builder().buffer().data()));

        auto num_atoms_before = fluent_state_atoms.count();

        // Create and all applicable actions and apply them
        // Attention: we cannot just apply newly generated actions because conditional effects might trigger later.
        delete_free_lifted_aag->generate_applicable_actions(state, actions);
        for (const auto& action : actions)
        {
            const auto succ_state = delete_free_ssg.get_or_create_successor_state(state, action);
            for (const auto atom_id : succ_state.get_atoms<Fluent>())
            {
                fluent_state_atoms.set(atom_id);
            }
        }

        // Create and all applicable axioms and apply them
        delete_free_lifted_aag->generate_and_apply_axioms(fluent_state_atoms, derived_state_atoms);

        auto num_atoms_after = fluent_state_atoms.count();

        if (num_atoms_before != num_atoms_after)
        {
            reached_delete_free_explore_fixpoint = false;
        }

    } while (!reached_delete_free_explore_fixpoint);

    m_event_handler->on_finish_delete_free_exploration(m_pddl_factories->get_ground_atoms_from_ids<Fluent>(fluent_state_atoms),
                                                       m_pddl_factories->get_ground_atoms_from_ids<Derived>(derived_state_atoms),
                                                       delete_free_lifted_aag->get_ground_actions(),
                                                       delete_free_lifted_aag->get_ground_axioms());

    auto fluent_ground_atoms_order = compute_ground_atom_order(m_pddl_factories->get_ground_atoms_from_ids<Fluent>(fluent_state_atoms), *m_pddl_factories);
    auto derived_ground_atoms_order = compute_ground_atom_order(m_pddl_factories->get_ground_atoms_from_ids<Derived>(derived_state_atoms), *m_pddl_factories);

    // 2. Create ground actions
    auto ground_actions = GroundActionList {};
    for (const auto& action : delete_free_lifted_aag->get_ground_actions())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_action : delete_relax_transformer.get_unrelaxed_actions(action.get_action()))
        {
            auto action_arguments = ObjectList(action.get_objects().begin(), action.get_objects().end());
            auto grounded_action = m_lifted_aag.ground_action(unrelaxed_action, std::move(action_arguments));
            if (grounded_action.is_statically_applicable(problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_actions.push_back(grounded_action);
            }
        }
    }

    m_event_handler->on_finish_grounding_unrelaxed_actions(ground_actions);

    // 3. Build match tree
    m_action_match_tree = MatchTree(ground_actions, fluent_ground_atoms_order, derived_ground_atoms_order);

    m_event_handler->on_finish_build_action_match_tree(m_action_match_tree);

    // 2. Create ground axioms
    auto ground_axioms = GroundAxiomList {};
    for (const auto& axiom : delete_free_lifted_aag->get_ground_axioms())
    {
        // Map relaxed to unrelaxed actions and ground them with the same arguments.
        for (const auto& unrelaxed_axiom : delete_relax_transformer.get_unrelaxed_axioms(axiom.get_axiom()))
        {
            auto axiom_arguments = ObjectList(axiom.get_objects().begin(), axiom.get_objects().end());
            auto grounded_axiom = m_lifted_aag.ground_axiom(unrelaxed_axiom, std::move(axiom_arguments));
            if (grounded_axiom.is_statically_applicable(problem->get_static_initial_positive_atoms_bitset()))
            {
                ground_axioms.push_back(grounded_axiom);
            }
        }
    }

    m_event_handler->on_finish_grounding_unrelaxed_axioms(ground_axioms);

    // 3. Build match tree
    m_axiom_match_tree = MatchTree(ground_axioms, fluent_ground_atoms_order, derived_ground_atoms_order);

    m_event_handler->on_finish_build_axiom_match_tree(m_axiom_match_tree);
}

void GroundedApplicableActionGenerator::generate_applicable_actions(State state, GroundActionList& out_applicable_actions)
{
    out_applicable_actions.clear();

    m_action_match_tree.get_applicable_elements(state.get_atoms<Fluent>(), state.get_atoms<Derived>(), out_applicable_actions);
}

void GroundedApplicableActionGenerator::generate_and_apply_axioms(const FlatBitsetBuilder<Fluent>& fluent_state_atoms,
                                                                  FlatBitsetBuilder<Derived>& ref_derived_state_atoms)
{
    for (const auto& lifted_partition : m_lifted_aag.get_axiom_partitioning())
    {
        bool reached_partition_fixed_point;

        do
        {
            reached_partition_fixed_point = true;

            /* Compute applicable axioms. */

            auto applicable_axioms = GroundAxiomList {};

            // TODO: For axioms, the same fluent branch is taken all the time.
            // Exploit this!
            m_axiom_match_tree.get_applicable_elements(fluent_state_atoms, ref_derived_state_atoms, applicable_axioms);

            /* Apply applicable axioms */

            for (const auto& grounded_axiom : applicable_axioms)
            {
                assert(grounded_axiom.get_axiom());
                if (!lifted_partition.get_axioms().count(grounded_axiom.get_axiom()))
                {
                    // axiom not part of same partition
                    continue;
                }

                assert(!grounded_axiom.get_derived_effect().is_negated);

                const auto grounded_atom_id = grounded_axiom.get_derived_effect().atom_id;

                if (!ref_derived_state_atoms.get(grounded_atom_id))
                {
                    // GENERATED NEW DERIVED ATOM!
                    reached_partition_fixed_point = false;
                }

                ref_derived_state_atoms.set(grounded_atom_id);
            }

        } while (!reached_partition_fixed_point);
    }
}

void GroundedApplicableActionGenerator::on_finish_f_layer() const { m_event_handler->on_finish_f_layer(); }

void GroundedApplicableActionGenerator::on_end_search() const { m_event_handler->on_end_search(); }

[[nodiscard]] const GroundActionList& GroundedApplicableActionGenerator::get_ground_actions() const { return m_lifted_aag.get_ground_actions(); }

[[nodiscard]] GroundAction GroundedApplicableActionGenerator::get_action(size_t action_id) const { return m_lifted_aag.get_action(action_id); }

Problem GroundedApplicableActionGenerator::get_problem() const { return m_problem; }

[[nodiscard]] const std::shared_ptr<PDDLFactories>& GroundedApplicableActionGenerator::get_pddl_factories() const { return m_pddl_factories; }

}
