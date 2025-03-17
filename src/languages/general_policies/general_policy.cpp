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

#include "mimir/languages/general_policies/general_policy.hpp"

#include "mimir/languages/general_policies/rule.hpp"
#include "mimir/languages/general_policies/visitor_interface.hpp"

namespace mimir::languages::general_policies
{

GeneralPolicy::GeneralPolicy(NamedFeatureLists<dl::Boolean, dl::Numerical> features, RuleList rules) : m_features(features), m_rules(rules) {}

bool GeneralPolicy::evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate(source_context, target_context); });
}

void GeneralPolicy::accept(IVisitor& visitor) { visitor.visit(*this); }

bool GeneralPolicy::is_terminating() const { throw std::runtime_error("Not implemented"); }

bool GeneralPolicy::solves(const datasets::GeneralizedStateSpace& generalized_state_space, const graphs::ClassGraph& graph)
{
    throw std::runtime_error("Not implemented");
}

const RuleList& GeneralPolicy::get_rules() const { return m_rules; }

}
