#include "mimir/search/applicable_action_generators/dense_lifted/event_handlers/default.hpp"

#include <iostream>

namespace mimir
{
void DefaultLiftedAAGEventHandler::on_start_generating_applicable_actions_impl() const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_action_impl(const Action action, const ObjectList& binding) const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_action_cache_hit_impl(const Action action, const ObjectList& binding) const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_action_cache_miss_impl(const Action action, const ObjectList& binding) const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_inapplicable_action_impl(const GroundAction action, const PDDLFactories& pddl_factories) const
{  //
}

void DefaultLiftedAAGEventHandler::on_end_generating_applicable_actions_impl(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_start_generating_applicable_axioms_impl() const
{  //
}

void DefaultLiftedAAGEventHandler::on_ground_axiom_impl(const Axiom axiom, const ObjectList& binding) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_ground_axiom_cache_hit_impl(const Axiom axiom, const ObjectList& binding) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_ground_axiom_cache_miss_impl(const Axiom axiom, const ObjectList& binding) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_ground_inapplicable_axiom_impl(const GroundAxiom axiom, const PDDLFactories& pddl_factories) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_end_generating_applicable_axioms_impl(const GroundAxiomList& ground_axioms, const PDDLFactories& pddl_factories) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_end_search_impl() const
{  //
    std::cout << get_statistics() << std::endl;
}
}