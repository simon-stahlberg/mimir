#include "../../include/mimir/search/search_base.hpp"

namespace planners
{
    SearchBase::SearchBase(const formalism::ProblemDescription& problem) :
        event_handlers_(),
        should_abort(false),
        initial_state(formalism::create_state(problem->initial, problem))
    {
    }

    void SearchBase::abort() { should_abort = true; }

    void SearchBase::set_initial_state(const formalism::State& state) { initial_state = state; }

    void SearchBase::notify_handlers() const
    {
        for (const auto& handler : event_handlers_)
        {
            handler();
        }
    }

    void SearchBase::register_handler(const std::function<void()>& handler) { event_handlers_.emplace_back(handler); }
}  // namespace planners
