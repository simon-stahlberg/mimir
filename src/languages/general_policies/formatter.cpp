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

#include "mimir/languages/general_policies/formatter.hpp"

#include "mimir/common/formatter.hpp"
#include "mimir/languages/general_policies/condition_interface.hpp"
#include "mimir/languages/general_policies/effect_interface.hpp"
#include "mimir/languages/general_policies/general_policy.hpp"
#include "mimir/languages/general_policies/named_feature.hpp"
#include "mimir/languages/general_policies/policy_graph.hpp"
#include "mimir/languages/general_policies/rule.hpp"
#include "mimir/languages/general_policies/visitor_formatter.hpp"

namespace mimir
{
namespace graphs
{
std::ostream& operator<<(std::ostream& out, const PolicyVertex& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const PolicyEdge& element) { return mimir::print(out, element); }
}  // end graphs

namespace languages::general_policies
{
std::ostream& operator<<(std::ostream& out, const ICondition& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const IEffect& element) { return mimir::print(out, element); }

template<dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& operator<<(std::ostream& out, const NamedFeatureImpl<D>& element)
{
    return mimir::print(out, element);
}

template std::ostream& operator<<(std::ostream& out, const NamedFeatureImpl<dl::ConceptTag>& element);
template std::ostream& operator<<(std::ostream& out, const NamedFeatureImpl<dl::RoleTag>& element);
template std::ostream& operator<<(std::ostream& out, const NamedFeatureImpl<dl::BooleanTag>& element);
template std::ostream& operator<<(std::ostream& out, const NamedFeatureImpl<dl::NumericalTag>& element);

std::ostream& operator<<(std::ostream& out, const RuleImpl& element) { return mimir::print(out, element); }

std::ostream& operator<<(std::ostream& out, const GeneralPolicyImpl& element) { return mimir::print(out, element); }

}  // end languages::general_policies

std::ostream& print(std::ostream& out, const mimir::graphs::PolicyVertex& element)
{
    fmt::print(out, "v_idx={}, conditions={}", element.get_index(), to_string(mimir::graphs::get_conditions(element)));
    return out;
}

std::ostream& print(std::ostream& out, const mimir::graphs::PolicyEdge& element)
{
    fmt::print(out, "e_idx={}, effects={}", element.get_index(), to_string(mimir::graphs::get_effects(element)));
    return out;
}

std::ostream& print(std::ostream& out, const mimir::languages::general_policies::ICondition& element)
{
    auto visitor = mimir::languages::general_policies::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::languages::general_policies::IEffect& element)
{
    auto visitor = mimir::languages::general_policies::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template<mimir::languages::dl::IsConceptOrRoleOrBooleanOrNumericalTag D>
std::ostream& print(std::ostream& out, const mimir::languages::general_policies::NamedFeatureImpl<D>& element)
{
    auto visitor = mimir::languages::general_policies::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

template std::ostream& print(std::ostream& out, const mimir::languages::general_policies::NamedFeatureImpl<mimir::languages::dl::ConceptTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::general_policies::NamedFeatureImpl<mimir::languages::dl::RoleTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::general_policies::NamedFeatureImpl<mimir::languages::dl::BooleanTag>& element);
template std::ostream& print(std::ostream& out, const mimir::languages::general_policies::NamedFeatureImpl<mimir::languages::dl::NumericalTag>& element);

std::ostream& print(std::ostream& out, const mimir::languages::general_policies::RuleImpl& element)
{
    auto visitor = mimir::languages::general_policies::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

std::ostream& print(std::ostream& out, const mimir::languages::general_policies::GeneralPolicyImpl& element)
{
    auto visitor = mimir::languages::general_policies::FormatterVisitor(out);
    element.accept(visitor);
    return out;
}

}
