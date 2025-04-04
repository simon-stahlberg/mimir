/*
 * Copyright (C) 2023 Dominik Drexler and Simon Stahlberg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "mimir/search/algorithms/iw/event_handlers/default.hpp"

#include "mimir/formalism/ground_action.hpp"
#include "mimir/search/plan.hpp"
#include "mimir/search/state.hpp"

using namespace mimir::formalism;

namespace mimir::search::iw
{
void DefaultEventHandlerImpl::on_start_search_impl(State initial_state) const {}

void DefaultEventHandlerImpl::on_start_arity_search_impl(State initial_state, size_t arity) const
{
    std::cout << "[IW] Start search with arity " << arity << std::endl;
}

void DefaultEventHandlerImpl::on_end_arity_search_impl(const brfs::Statistics& brfs_statistics) const {}

void DefaultEventHandlerImpl::on_end_search_impl() const { std::cout << "[IW] Search ended.\n" << m_statistics << std::endl; }

void DefaultEventHandlerImpl::on_solved_impl(const Plan& plan) const
{
    std::cout << "[IW] Plan found.\n"
              << "[IW] Plan cost: " << plan.get_cost() << "\n"
              << "[IW] Plan length: " << plan.get_actions().size() << std::endl;
    for (size_t i = 0; i < plan.get_actions().size(); ++i)
    {
        std::cout << "[IW] " << i + 1 << ". ";
        mimir::operator<<(std::cout, std::make_tuple(plan.get_actions()[i], std::cref(*m_problem), GroundActionImpl::PlanFormatterTag {}));
        std::cout << std::endl;
    }
}

void DefaultEventHandlerImpl::on_unsolvable_impl() const { std::cout << "[IW] Unsolvable!" << std::endl; }

void DefaultEventHandlerImpl::on_exhausted_impl() const { std::cout << "[IW] Exhausted!" << std::endl; }

DefaultEventHandlerImpl::DefaultEventHandlerImpl(Problem problem, bool quiet) : EventHandlerBase<DefaultEventHandlerImpl>(problem, quiet) {}

DefaultEventHandler DefaultEventHandlerImpl::create(formalism::Problem problem, bool quiet)
{
    return std::make_shared<DefaultEventHandlerImpl>(problem, quiet);
}
}
