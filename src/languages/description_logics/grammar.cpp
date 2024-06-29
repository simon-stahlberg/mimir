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

#include "parser.hpp"

#include <mimir/languages/description_logics/grammar.hpp>

namespace mimir::dl::grammar
{

/**
 * NonTerminal
 */

template<dl::IsConceptOrRole D>
NonTerminal<D>::NonTerminal(size_t id, const DerivationRule<D>& rule) : m_id(id), m_rule(rule)
{
}

template<dl::IsConceptOrRole D>
bool NonTerminal<D>::test_match(const D& constructor) const
{
    return m_rule.test_match(constructor);
}

template<dl::IsConceptOrRole D>
size_t NonTerminal<D>::get_id() const
{
    return m_id;
}

/**
 * DerivationRule
 */

template<dl::IsConceptOrRole D>
DerivationRule<D>::DerivationRule(size_t id, std::vector<Choice<D>> choices) : m_id(id), m_choices(std::move(choices))
{
}

template<dl::IsConceptOrRole D>
bool DerivationRule<D>::test_match(const D& constructor) const
{
    return std::any_of(m_choices.begin(),
                       m_choices.end(),
                       [&constructor](const Choice<D>& choice)
                       { return std::visit([&constructor](const auto& arg) -> bool { return arg.get().test_match(constructor); }, choice); });
}

template<dl::IsConceptOrRole D>
size_t DerivationRule<D>::get_id() const
{
    return m_id;
}

/**
 * Grammar
 */

Grammar::Grammar(std::string bnf_description)
{
    const auto [concept_rules, role_rules] = parse(bnf_description, m_grammar_constructor_repos);
    m_concept_rules = std::move(concept_rules);
    m_role_rules = std::move(role_rules);
}

bool Grammar::test_match(const dl::Concept& constructor) const
{
    return std::any_of(m_concept_rules.begin(),
                       m_concept_rules.end(),
                       [&constructor](const DerivationRule<Concept>* rule) { return rule->test_match(constructor); });
}

bool Grammar::test_match(const dl::Role& constructor) const
{
    return std::any_of(m_role_rules.begin(), m_role_rules.end(), [&constructor](const DerivationRule<Role>* rule) { return rule->test_match(constructor); });
}

}
