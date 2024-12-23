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
#include "mimir/formalism/effects.hpp"
#include "mimir/formalism/existentially_quantified_conjunctive_condition.hpp"
#include "mimir/formalism/function_expressions.hpp"
#include "mimir/formalism/literal.hpp"
#include "mimir/formalism/predicate.hpp"
#include "mimir/formalism/variable.hpp"

#include <cassert>

namespace mimir
{
ActionImpl::ActionImpl(Index index,
                       std::string name,
                       size_t original_arity,
                       ExistentiallyQuantifiedConjunctiveCondition precondition,
                       EffectStrips strips_effect,
                       EffectConditionalList conditional_effects) :
    m_index(index),
    m_name(std::move(name)),
    m_original_arity(original_arity),
    m_precondition(std::move(precondition)),
    m_strips_effect(std::move(strips_effect)),
    m_conditional_effects(std::move(conditional_effects))
{
    assert(is_all_unique(m_conditional_effects));
    assert(std::is_sorted(m_conditional_effects.begin(),
                          m_conditional_effects.end(),
                          [](const auto& l, const auto& r) { return l->get_index() < r->get_index(); }));
}

Index ActionImpl::get_index() const { return m_index; }

const std::string& ActionImpl::get_name() const { return m_name; }

size_t ActionImpl::get_original_arity() const { return m_original_arity; }

const VariableList& ActionImpl::get_parameters() const { return m_precondition->get_parameters(); }

const ExistentiallyQuantifiedConjunctiveCondition& ActionImpl::get_precondition() const { return m_precondition; }

const EffectStrips& ActionImpl::get_strips_effect() const { return m_strips_effect; }

const EffectConditionalList& ActionImpl::get_conditional_effects() const { return m_conditional_effects; }

size_t ActionImpl::get_arity() const { return get_parameters().size(); }

std::ostream& operator<<(std::ostream& out, const ActionImpl& element)
{
    auto formatter = PDDLFormatter();
    formatter.write(element, out);
    return out;
}

std::ostream& operator<<(std::ostream& out, Action element)
{
    out << *element;
    return out;
}

}
