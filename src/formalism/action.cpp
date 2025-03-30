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

#include "mimir/formalism/action.hpp"

#include "formatter.hpp"
#include "mimir/common/collections.hpp"
#include "mimir/common/concepts.hpp"
#include "mimir/common/printers.hpp"
#include "mimir/formalism/atom.hpp"
#include "mimir/formalism/conjunctive_condition.hpp"
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <absl/container/flat_hash_map.h>
#include <cassert>

namespace mimir::formalism
{

static void validate_non_conflicting_numeric_effects(ConjunctiveEffect conjunctive_effect,
                                                     absl::flat_hash_map<Function<FluentTag>, loki::AssignOperatorEnum>& ref_fluent_function_to_change,
                                                     std::optional<loki::AssignOperatorEnum>& ref_auxiliary_function_change)
{
    for (const auto& numeric_effect : conjunctive_effect->get_fluent_numeric_effects())
    {
        auto it = ref_fluent_function_to_change.find(numeric_effect->get_function());
        if (it != ref_fluent_function_to_change.end())
        {
            if (it->second != numeric_effect->get_assign_operator())
            {
                throw std::runtime_error(
                    "validate_non_conflicting_numeric_effects(conjunctive_effect, ref_fluent_function_to_change, ref_auxiliary_function_change): "
                    "detected conflicting fluent numeric effects of a lifted action schema that changes it in a different direction "
                    + to_string(numeric_effect));
            }
            // Note: we are a bit softer here. According to PDDL, multiple SCALE_UP and SCALE_DOWN are conflicting
            else if (it->second == loki::AssignOperatorEnum::ASSIGN)
            {
                throw std::runtime_error(
                    "validate_non_conflicting_numeric_effects(conjunctive_effect, ref_fluent_function_to_change, ref_auxiliary_function_change): "
                    "detected conflicting fluent numeric effects of a lifted action schema that reassigns it "
                    + to_string(numeric_effect));
            }
        }
        else
        {
            ref_fluent_function_to_change[numeric_effect->get_function()] = numeric_effect->get_assign_operator();
        }
    }

    if (conjunctive_effect->get_auxiliary_numeric_effect().has_value())
    {
        const auto& auxiliary_numeric_effect = conjunctive_effect->get_auxiliary_numeric_effect().value();

        if (ref_auxiliary_function_change.has_value())
        {
            if (ref_auxiliary_function_change.value() != auxiliary_numeric_effect->get_assign_operator())
            {
                throw std::runtime_error(
                    "validate_non_conflicting_numeric_effects(conjunctive_effect, ref_fluent_function_to_change, ref_auxiliary_function_change): "
                    "detected conflicting auxiliary numeric effects of a lifted action schema that changes it in a different direction "
                    + to_string(auxiliary_numeric_effect));
            }
            // Note: we are a bit softer here. According to PDDL, multiple SCALE_UP and SCALE_DOWN are conflicting
            else if (ref_auxiliary_function_change.value() == loki::AssignOperatorEnum::ASSIGN)
            {
                throw std::runtime_error(
                    "validate_non_conflicting_numeric_effects(conjunctive_effect, ref_fluent_function_to_change, ref_auxiliary_function_change): "
                    "detected conflicting auxiliary numeric effects of a lifted action schema that reassigns it "
                    + to_string(auxiliary_numeric_effect));
            }
        }
        else
        {
            ref_auxiliary_function_change = auxiliary_numeric_effect->get_assign_operator();
        }
    }
}

/// @brief Validate that the lifted effects are non-conflicting.
/// This approximates the test on the grounded level but should usually suffice.
/// To get a more exact test, we could specifically store the effects that might conflict in the grounded case, and then test those in the applicability check.
/// Axioms could also help here but I want to avoid introducing additional derived variables.
/// @param conjunctive_effect
/// @param conditional_effects
/// @return
static void validate_non_conflicting_numeric_effects(ConjunctiveEffect conjunctive_effect, const ConditionalEffectList& conditional_effects)
{
    auto fluent_function_to_change = absl::flat_hash_map<Function<FluentTag>, loki::AssignOperatorEnum> {};
    auto auxiliary_function_change = std::optional<loki::AssignOperatorEnum> { std::nullopt };

    validate_non_conflicting_numeric_effects(conjunctive_effect, fluent_function_to_change, auxiliary_function_change);

    for (const auto& conditional_effect : conditional_effects)
    {
        validate_non_conflicting_numeric_effects(conditional_effect->get_conjunctive_effect(), fluent_function_to_change, auxiliary_function_change);
    }
}

ActionImpl::ActionImpl(Index index,
                       std::string name,
                       size_t original_arity,
                       ConjunctiveCondition conjunctive_condition,
                       ConjunctiveEffect conjunctive_effect,
                       ConditionalEffectList conditional_effects) :
    m_index(index),
    m_name(std::move(name)),
    m_original_arity(original_arity),
    m_conjunctive_condition(std::move(conjunctive_condition)),
    m_conjunctive_effect(std::move(conjunctive_effect)),
    m_conditional_effects(std::move(conditional_effects))
{
    assert(m_conjunctive_condition);
    assert(m_conjunctive_effect);
    assert(is_all_unique(m_conditional_effects));
    assert(std::is_sorted(m_conditional_effects.begin(),
                          m_conditional_effects.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));

    // Note: we test on the lifted level as a safety net for now...
    validate_non_conflicting_numeric_effects(m_conjunctive_effect, m_conditional_effects);
}

Index ActionImpl::get_index() const { return m_index; }

const std::string& ActionImpl::get_name() const { return m_name; }

size_t ActionImpl::get_original_arity() const { return m_original_arity; }

const VariableList& ActionImpl::get_parameters() const { return m_conjunctive_condition->get_parameters(); }

ConjunctiveCondition ActionImpl::get_conjunctive_condition() const { return m_conjunctive_condition; }

ConjunctiveEffect ActionImpl::get_conjunctive_effect() const { return m_conjunctive_effect; }

const ConditionalEffectList& ActionImpl::get_conditional_effects() const { return m_conditional_effects; }

size_t ActionImpl::get_arity() const { return get_parameters().size(); }

std::ostream& operator<<(std::ostream& out, const ActionImpl& element)
{
    write(element, StringFormatter(), out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Action element)
{
    write(*element, AddressFormatter(), out);
    return out;
}

}
