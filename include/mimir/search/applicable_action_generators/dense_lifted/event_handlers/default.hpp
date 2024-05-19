#ifndef MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_DEFAULT_HPP_
#define MIMIR_SEARCH_APPLICABLE_ACTION_GENERATORS_DENSE_LIFTED_EVENT_HANDLERS_DEFAULT_HPP_

#include "mimir/search/applicable_action_generators/dense_lifted/event_handlers/interface.hpp"

namespace mimir
{

class DefaultLiftedAAGEventHandler : public LiftedAAGEventHandlerBase<DefaultLiftedAAGEventHandler>
{
private:
    /* Implement LiftedAAGEventHandlerBase interface */
    friend class LiftedAAGEventHandlerBase<DefaultLiftedAAGEventHandler>;

    void on_start_generating_applicable_actions_impl() const;

    void on_ground_action_impl(const Action action, const ObjectList& binding) const;

    void on_ground_action_cache_hit_impl(const Action action, const ObjectList& binding) const;

    void on_ground_action_cache_miss_impl(const Action action, const ObjectList& binding) const;

    void on_ground_inapplicable_action_impl(const GroundAction action, const PDDLFactories& pddl_factories) const;

    void on_end_generating_applicable_actions_impl(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) const;

    void on_start_generating_applicable_axioms_impl() const;

    void on_ground_axiom_impl(const Axiom axiom, const ObjectList& binding) const;

    void on_ground_axiom_cache_hit_impl(const Axiom axiom, const ObjectList& binding) const;

    void on_ground_axiom_cache_miss_impl(const Axiom axiom, const ObjectList& binding) const;

    void on_ground_inapplicable_axiom_impl(const GroundAxiom axiom, const PDDLFactories& pddl_factories) const;

    void on_end_generating_applicable_axioms_impl(const GroundAxiomList& ground_axioms, const PDDLFactories& pddl_factories) const;

    void on_finish_f_layer_impl() const;

    void on_end_search_impl() const;
};

}

#endif