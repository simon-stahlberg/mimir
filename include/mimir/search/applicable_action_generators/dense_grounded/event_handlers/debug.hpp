#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_GROUNDED_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/search/applicable_action_generators/dense_grounded/event_handlers/interface.hpp"

namespace mimir
{

class DebugGroundedAAGEventHandler : public GroundedAAGEventHandlerBase<DebugGroundedAAGEventHandler>
{
private:
    /* Implement GroundedAAGEventHandlerBase interface */
    friend class GroundedAAGEventHandlerBase<DebugGroundedAAGEventHandler>;

    void on_finish_delete_free_exploration_impl(const GroundAtomList& reached_atoms,
                                                const GroundActionList& instantiated_actions,
                                                const GroundAxiomList& instantiated_axioms);

    void on_finish_grounding_unrelaxed_actions_impl(const GroundActionList& unrelaxed_actions);

    void on_finish_build_action_match_tree_impl(const MatchTree<GroundAction>& action_match_tree);

    void on_finish_grounding_unrelaxed_axioms_impl(const GroundAxiomList& unrelaxed_axioms);

    void on_finish_build_axiom_match_tree_impl(const MatchTree<GroundAxiom>& axiom_match_tree);

    void on_finish_f_layer_impl() const;

    void on_end_search_impl() const;
};

}

#endif