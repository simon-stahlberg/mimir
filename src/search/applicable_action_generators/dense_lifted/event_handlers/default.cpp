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
    std::cout << std::make_tuple(action, std::cref(pddl_factories)) << std::endl;
    exit(1);
}

void DefaultLiftedAAGEventHandler::on_end_generating_applicable_actions_impl(const GroundActionList& ground_actions, const PDDLFactories& pddl_factories) const
{
    //
}

void DefaultLiftedAAGEventHandler::on_end_search_impl() const
{  //
    std::cout << get_statistics() << std::endl;
}
}