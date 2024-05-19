#include "mimir/search/algorithms/event_handlers/default.hpp"

#include "mimir/common/printers.hpp"
#include "mimir/search/plan.hpp"

namespace mimir
{
void DefaultAlgorithmEventHandler::on_generate_state_impl(GroundAction action, State successor_state, const PDDLFactories& pddl_factories) const {}

void DefaultAlgorithmEventHandler::on_finish_g_layer_impl(uint64_t g_value, uint64_t num_states) const
{
    std::cout << "[Algorithm] Finished state expansion until g-layer " << g_value << " with num states " << num_states << std::endl;
}

void DefaultAlgorithmEventHandler::on_expand_state_impl(State state, const PDDLFactories& pddl_factories) const {}

void DefaultAlgorithmEventHandler::on_start_search_impl(State initial_state, const PDDLFactories& pddl_factories) const
{  //
    std::cout << "[Algorithm] Search started." << std::endl;
}

void DefaultAlgorithmEventHandler::on_end_search_impl() const
{
    std::cout << "[Algorithm] Search ended."
              << "\n"
              << m_statistics << std::endl;
}

void DefaultAlgorithmEventHandler::on_solved_impl(const GroundActionList& ground_action_plan) const
{
    auto plan = to_plan(ground_action_plan);
    std::cout << "[Algorithm] Plan found with cost: " << plan.get_cost() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[Algorithm] " << i + 1 << ". " << plan.get_actions()[i] << std::endl;
    }
}

void DefaultAlgorithmEventHandler::on_exhausted_impl() const { std::cout << "[Algorithm] Exhausted!" << std::endl; }
}
