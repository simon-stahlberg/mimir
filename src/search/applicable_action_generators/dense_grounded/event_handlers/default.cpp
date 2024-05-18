#include "mimir/search/applicable_action_generators/dense_grounded/event_handlers/default.hpp"

#include "mimir/search/applicable_action_generators/dense_grounded/match_tree.hpp"

#include <iostream>

namespace mimir
{

void DefaultGroundedAAGEventHandler::on_finish_delete_free_exploration_impl(const GroundAtomList& reached_atoms,
                                                                            const GroundActionList& instantiated_actions,
                                                                            const GroundAxiomList& instantiated_axioms)
{
    std::cout << "[Grounded AAG] Total number of ground atoms reachable in delete-relaxed task: " << reached_atoms.size() << "\n"
              << "[Grounded AAG] Total number of instantiated delete free ground actions: " << instantiated_actions.size() << "\n"
              << "[Grounded AAG] Total number of instantiated delete free ground axioms: " << instantiated_axioms.size() << std::endl;
}

void DefaultGroundedAAGEventHandler::on_finish_grounding_unrelaxed_actions_impl(const GroundActionList& unrelaxed_actions)
{
    std::cout << "[Grounded AAG] Total number of ground actions in task: " << unrelaxed_actions.size() << std::endl;
}

void DefaultGroundedAAGEventHandler::on_finish_build_action_match_tree_impl(const MatchTree<GroundAction>& action_match_tree)
{
    std::cout << "[Grounded AAG] Total number of nodes in action match tree: " << action_match_tree.get_num_nodes() << std::endl;
}

void DefaultGroundedAAGEventHandler::on_finish_grounding_unrelaxed_axioms_impl(const GroundAxiomList& unrelaxed_axioms)
{
    std::cout << "[Grounded AAG] Total number of ground axioms in task: " << unrelaxed_axioms.size() << std::endl;
}

void DefaultGroundedAAGEventHandler::on_finish_build_axiom_match_tree_impl(const MatchTree<GroundAxiom>& axiom_match_tree)
{
    std::cout << "[Grounded AAG] Total number of nodes in axiom match tree: " << axiom_match_tree.get_num_nodes() << std::endl;
}

}