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

GeneralPolicyImpl::GeneralPolicyImpl(Index index, NamedFeatureLists<dl::Boolean, dl::Numerical> features, RuleList rules) :
    m_index(index),
    m_features(features),
    m_rules(rules)
{
}

bool GeneralPolicyImpl::evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate(source_context, target_context); });
}

void GeneralPolicyImpl::accept(IVisitor& visitor) { visitor.visit(this); }

bool GeneralPolicyImpl::is_terminating() const { throw std::runtime_error("Not implemented"); }

bool GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space, const graphs::ClassGraph& graph)
{
    throw std::runtime_error("Not implemented");
}

Index GeneralPolicyImpl::get_index() const { return m_index; }

template<dl::FeatureCategory D>
const NamedFeatureList<D>& GeneralPolicyImpl::get_features() const
{
    return boost::hana::at_key(m_features, boost::hana::type<D> {});
}

template const NamedFeatureList<dl::Boolean>& GeneralPolicyImpl::get_features() const;
template const NamedFeatureList<dl::Numerical>& GeneralPolicyImpl::get_features() const;

const NamedFeatureLists<dl::Boolean, dl::Numerical>& GeneralPolicyImpl::get_hana_features() const { return m_features; }

const RuleList& GeneralPolicyImpl::get_rules() const { return m_rules; }

}
