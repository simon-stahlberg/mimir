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
#include "mimir/search/event_handlers/minimal.hpp"
#include "mimir/search/successor_state_generators/dense.hpp"

#include <deque>

namespace mimir
{

MatchTree::NodeID MatchTree::build_recursively(const size_t atom_id, const size_t num_atoms, const std::vector<ConstDenseActionViewProxy>& actions)
{
    // 1. Base cases:

    // 1.1. There are no more atoms to test or
    // 1.2. there are no actions.
    if ((atom_id == num_atoms) || (actions.empty()))
    {
        const auto node_id = MatchTree::NodeID { m_nodes.size() };
        m_nodes.push_back(MatchTree::GeneratorNode { m_actions.size(), m_actions.size() + actions.size() });
        m_actions.insert(m_actions.end(), actions.begin(), actions.end());

        return node_id;
    }

    // 2. Conquer

    // Partition actions into positive, negative and dontcare depending on how atom_id occurs in precondition
    auto positive_actions = std::vector<ConstDenseActionViewProxy> {};
    auto negative_actions = std::vector<ConstDenseActionViewProxy> {};
    auto dontcare_actions = std::vector<ConstDenseActionViewProxy> {};
    for (const auto& action : actions)
    {
        const bool positive_condition = action.get_applicability_positive_precondition_bitset().get(atom_id);
        const bool negative_condition = action.get_applicability_negative_precondition_bitset().get(atom_id);
        assert(!(positive_condition && negative_condition));

        if (negative_condition)
        {
            negative_actions.push_back(action);
        }
        else if (positive_condition)
        {
            positive_actions.push_back(action);
        }
        else
        {
            dontcare_actions.push_back(action);
        }
    }

    // 2. Inductive cases:
    const bool must_split = (dontcare_actions.size() != actions.size());
    if (must_split)
    {
        // Top-down creation of nodes, update information after recursion.
        const auto node_id = MatchTree::NodeID { m_nodes.size() };
        m_nodes.push_back(MatchTree::SelectorNode { atom_id, 0, 0, 0 });

        const auto true_succ = build_recursively(atom_id + 1, num_atoms, positive_actions);
        const auto false_succ = build_recursively(atom_id + 1, num_atoms, negative_actions);
        const auto dontcare_succ = build_recursively(atom_id + 1, num_atoms, dontcare_actions);

        assert(node_id < true_succ);
        assert(node_id < false_succ);
        assert(node_id < dontcare_succ);

        // Update node with computed information
        auto& node = std::get<MatchTree::SelectorNode>(m_nodes[node_id]);
        node.true_succ = true_succ;
        node.false_succ = false_succ;
        node.dontcare_succ = dontcare_succ;

        return node_id;
    }
    else
    {
        // All actions are dontcares, skip creating a node.
        return build_recursively(atom_id + 1, num_atoms, dontcare_actions);
    }
}

size_t MatchTree::get_num_nodes() const { return m_nodes.size(); }

MatchTree::MatchTree() { m_nodes.push_back(MatchTree::GeneratorNode { 0, 0 }); }

MatchTree::MatchTree(const size_t num_atoms, const std::vector<ConstDenseActionViewProxy>& actions)
{
    assert(m_nodes.size() == 0);
    const auto root_node_id = build_recursively(m_nodes.size(), num_atoms, actions);
    assert(root_node_id == 0);
}

void MatchTree::get_applicable_actions_recursively(size_t node_id,
                                                   const ConstDenseStateViewProxy state,
                                                   std::vector<ConstDenseActionViewProxy>& out_applicable_actions)
{
    auto& node = m_nodes[node_id];

    if (const auto generator_node = std::get_if<MatchTree::GeneratorNode>(&node))
    {
        out_applicable_actions.insert(out_applicable_actions.end(), m_actions.begin() + generator_node->begin, m_actions.begin() + generator_node->end);
    }
    else if (const auto selector_node = std::get_if<MatchTree::SelectorNode>(&node))
    {
        get_applicable_actions_recursively(selector_node->dontcare_succ, state, out_applicable_actions);

        if (state.get_atoms_bitset().get(selector_node->ground_atom_id))
        {
            get_applicable_actions_recursively(selector_node->true_succ, state, out_applicable_actions);
        }
        else
        {
            get_applicable_actions_recursively(selector_node->false_succ, state, out_applicable_actions);
        }
    }
}

void MatchTree::get_applicable_actions(const ConstDenseStateViewProxy state, std::vector<ConstDenseActionViewProxy>& out_applicable_actions)
{
    out_applicable_actions.clear();

    assert(!m_nodes.empty());

    get_applicable_actions_recursively(0, state, out_applicable_actions);
}

AAG<GroundedAAGDispatcher<DenseStateTag>>::AAG(Problem problem, PDDLFactories& pddl_factories) :
    m_problem(problem),
    m_pddl_factories(pddl_factories),
    m_lifted_aag(problem, pddl_factories)
{
    // 1. Explore delete relaxed task.
    auto delete_relax_transformer = DeleteRelaxTransformer(m_pddl_factories);
    const auto dr_problem = delete_relax_transformer.run(*m_problem);
    auto dr_lifted_aag = AAG<LiftedAAGDispatcher<DenseStateTag>>(dr_problem, m_pddl_factories);
    auto dr_ssg = SSG<SSGDispatcher<DenseStateTag>>(dr_problem);

    auto& state_bitset = m_state_builder.get_atoms_bitset();
    state_bitset.unset_all();

    // Keep track of changes
    size_t num_atoms = 0;
    size_t num_actions = 0;
    // Temporary variables
    auto actions = std::vector<ConstDenseActionViewProxy> {};
    do
    {
        num_atoms = pddl_factories.get_ground_atoms().size();
        num_actions = dr_lifted_aag.get_actions().size();

        // Create a state where all ground atoms are true
        for (const auto& atom : pddl_factories.get_ground_atoms())
        {
            state_bitset.set(atom.get_identifier());
        }
        m_state_builder.get_flatmemory_builder().finish();
        const auto state = ConstDenseStateViewProxy(ConstDenseStateView(m_state_builder.get_flatmemory_builder().buffer().data()));

        // Create all applicable actions and apply newly generated actions
        dr_lifted_aag.generate_applicable_actions(state, actions);
        for (const auto& action : actions)
        {
            const auto is_newly_generated = (action.get_id() >= num_actions);
            if (is_newly_generated)
            {
                (void) dr_ssg.get_or_create_successor_state(state, action);
            }
        }

    } while (num_atoms != pddl_factories.get_ground_atoms().size());

    std::cout << "Total number of ground atoms reachable in delete-relaxed task: " << m_pddl_factories.get_ground_atoms().size() << std::endl;
    std::cout << "Total number of ground actions in delete-relaxed task: " << dr_lifted_aag.get_actions().size() << std::endl;

    // 2. Create ground actions
    const auto state = ConstDenseStateViewProxy(ConstDenseStateView(m_state_builder.get_flatmemory_builder().buffer().data()));
    m_lifted_aag.generate_applicable_actions(state, actions);

    std::cout << "Total number of ground actions in task: " << m_lifted_aag.get_actions().size() << std::endl;

    // 3. Build match tree
    m_match_tree = MatchTree(m_pddl_factories.get_ground_atoms().size(), actions);

    std::cout << "Total number of nodes in match tree: " << m_match_tree.get_num_nodes() << std::endl;
}

void AAG<GroundedAAGDispatcher<DenseStateTag>>::generate_applicable_actions_impl(ConstStateView state, std::vector<ConstActionView>& out_applicable_actions)
{
    out_applicable_actions.clear();

    m_match_tree.get_applicable_actions(state, out_applicable_actions);
}

[[nodiscard]] ConstView<ActionDispatcher<DenseStateTag>> AAG<GroundedAAGDispatcher<DenseStateTag>>::get_action(size_t action_id) const
{
    return m_lifted_aag.get_action(action_id);
}
}
