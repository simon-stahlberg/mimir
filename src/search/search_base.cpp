#include "../../include/mimir/search/search_base.hpp"

namespace mimir::planners
{
    SearchBase::SearchBase(const mimir::formalism::ProblemDescription& problem) :
        event_handlers_(),
        initial_state(mimir::formalism::create_state(problem->initial, problem)),
        should_abort(false)
    {
    }

    void SearchBase::abort() { should_abort = true; }

    void SearchBase::set_initial_state(const mimir::formalism::State& state) { initial_state = state; }

    void SearchBase::notify_handlers() const
    {
        for (const auto& handler : event_handlers_)
        {
            handler();
        }
    }

    void SearchBase::register_handler(const std::function<void()>& handler) { event_handlers_.emplace_back(handler); }
}  // namespace planners
