#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_DEBUG_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_DEBUG_HPP_

#include "mimir/search/applicable_action_generators/dense_lifted/event_handlers/interface.hpp"

namespace mimir
{

class DebugLiftedAAGEventHandler : public LiftedAAGEventHandlerBase<DebugLiftedAAGEventHandler>
{
private:
    /* Implement LiftedAAGEventHandlerBase interface */
    friend class LiftedAAGEventHandlerBase<DebugLiftedAAGEventHandler>;

    void on_start_generating_applicable_actions_impl() const;

    void on_ground_action_impl(const Action action, const ObjectList& binding) const;

    void on_ground_action_cache_hit_impl(const Action action, const ObjectList& binding) const;

    void on_ground_action_cache_miss_impl(const Action action, const ObjectList& binding) const;

    void on_ground_inapplicable_action_impl(const GroundAction action, const PDDLFactories& pddl_factories) const;

    void on_end_generating_applicable_actions_impl(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) const;

    void on_end_search_impl() const;
};

}

#endif