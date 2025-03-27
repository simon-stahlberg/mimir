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
#include "mimir/search/state.hpp"

namespace mimir::languages::general_policies
{

GeneralPolicyImpl::GeneralPolicyImpl(Index index, NamedFeatureLists<dl::BooleanTag, dl::NumericalTag> features, RuleList rules) :
    m_index(index),
    m_features(features),
    m_rules(rules)
{
}

bool GeneralPolicyImpl::evaluate(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate(source_context, target_context); });
}

bool GeneralPolicyImpl::evaluate_with_debug(dl::EvaluationContext& source_context, dl::EvaluationContext& target_context) const
{
    return std::any_of(get_rules().begin(), get_rules().end(), [&](auto&& arg) { return arg->evaluate_with_debug(source_context, target_context); });
}

void GeneralPolicyImpl::accept(IVisitor& visitor) { visitor.visit(this); }

bool GeneralPolicyImpl::is_terminating() const { throw std::runtime_error("Not implemented"); }

GeneralPolicyImpl::UnsolvabilityReason GeneralPolicyImpl::solves(const datasets::StateSpace& state_space,
                                                                 dl::DenotationRepositories& denotation_repositories) const
{
    auto alive_vertex_indices = graphs::VertexIndexList {};
    for (const auto& vertex : state_space->get_graph().get_vertices())
    {
        if (is_alive(vertex))
            alive_vertex_indices.push_back(vertex.get_index());
    }

    return solves(state_space, alive_vertex_indices, denotation_repositories);
}

GeneralPolicyImpl::UnsolvabilityReason GeneralPolicyImpl::solves(const datasets::StateSpace& state_space,
                                                                 const graphs::VertexIndexList& vertices,
                                                                 dl::DenotationRepositories& denotation_repositories) const
{
    return solves<graphs::VertexIndexList>(state_space, vertices, denotation_repositories);
}

GeneralPolicyImpl::UnsolvabilityReason GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                 dl::DenotationRepositories& denotation_repositories) const
{
    auto alive_vertex_indices = graphs::VertexIndexList {};
    for (const auto& vertex : generalized_state_space->get_graph().get_vertices())
    {
        if (is_alive(generalized_state_space->get_problem_vertex(vertex)))
            alive_vertex_indices.push_back(vertex.get_index());
    }

    return solves(generalized_state_space, alive_vertex_indices, denotation_repositories);
}

GeneralPolicyImpl::UnsolvabilityReason GeneralPolicyImpl::solves(const datasets::GeneralizedStateSpace& generalized_state_space,
                                                                 const graphs::VertexIndexList& vertices,
                                                                 dl::DenotationRepositories& denotation_repositories) const
{
    return solves<graphs::VertexIndexList>(generalized_state_space, vertices, denotation_repositories);
}

Index GeneralPolicyImpl::get_index() const { return m_index; }

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
const NamedFeatureList<D>& GeneralPolicyImpl::get_features() const
{
    return boost::hana::at_key(m_features, boost::hana::type<D> {});
}

template const NamedFeatureList<dl::BooleanTag>& GeneralPolicyImpl::get_features() const;
template const NamedFeatureList<dl::NumericalTag>& GeneralPolicyImpl::get_features() const;

const NamedFeatureLists<dl::BooleanTag, dl::NumericalTag>& GeneralPolicyImpl::get_hana_features() const { return m_features; }

const RuleList& GeneralPolicyImpl::get_rules() const { return m_rules; }
}
