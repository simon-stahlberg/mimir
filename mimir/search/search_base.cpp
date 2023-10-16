#include "search_base.hpp"

namespace planners
{
    SearchBase::SearchBase() : event_handlers_(), should_abort(false) {}

    void SearchBase::abort() { should_abort = true; }

    void SearchBase::notify_handlers() const
    {
        for (const auto& handler : event_handlers_)
        {
            handler();
        }
    }

    void SearchBase::register_handler(const std::function<void()>& handler) { event_handlers_.emplace_back(handler); }
}  // namespace planners
