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
 * Grammar
 */

Grammar::Grammar(std::string bnf_description, Domain domain)
{
    const auto [concept_rules, role_rules] = parse(bnf_description, domain, m_grammar_constructor_repos);
    m_concept_rules = std::move(concept_rules);
    m_role_rules = std::move(role_rules);
}

bool Grammar::test_match(const dl::Constructor<Concept>& constructor) const
{
    return std::any_of(m_concept_rules.begin(), m_concept_rules.end(), [&constructor](const auto& rule) { return rule.get().test_match(constructor); });
}

bool Grammar::test_match(const dl::Constructor<Role>& constructor) const
{
    return std::any_of(m_role_rules.begin(), m_role_rules.end(), [&constructor](const auto& rule) { return rule.get().test_match(constructor); });
}

const ConceptDerivationRuleList& Grammar::get_concept_rules() const { return m_concept_rules; }

const RoleDerivationRuleList& Grammar::get_role_rules() const { return m_role_rules; }

}
