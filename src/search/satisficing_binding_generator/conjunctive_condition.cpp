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

#include "mimir/search/satisficing_binding_generators/conjunctive_condition.hpp"

#include "mimir/search/satisficing_binding_generators/base_impl.hpp"

using namespace mimir::formalism;

namespace mimir::search
{

ConjunctiveConditionSatisficingBindingGenerator::ConjunctiveConditionSatisficingBindingGenerator(ConjunctiveCondition conjunctive_condition,
                                                                                                 Problem problem,
                                                                                                 std::shared_ptr<IEventHandler> event_handler) :
    SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>(conjunctive_condition, std::move(problem), event_handler)
{
}

template class SatisficingBindingGenerator<ConjunctiveConditionSatisficingBindingGenerator>;
}
